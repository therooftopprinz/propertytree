#include <list>

#include <bfc/ThreadPool.hpp>
#include <bfc/Timer.hpp>
#include <bfc/Singleton.hpp>
#include <bfc/Buffer.hpp>

#include <logless/Logger.hpp>

#include <interface/protocol.hpp>

#include <IConnectionSession.hpp>

#include <ProtocolHandler.hpp>

extern Logger logger;

namespace propertytree
{

constexpr size_t ENCODE_SIZE = 1024*64;

ProtocolHandler::ProtocolHandler(bfc::LightFn<void()> pTerminator)
    : mTerminator(pTerminator)
{
    auto rootUUid = mUuidCtr++;

    mTree.emplace(rootUUid, std::make_shared<Node>("", 0xFFFFFFFF, std::weak_ptr<Node>(), rootUUid));
}

void ProtocolHandler::onDisconnect(IConnectionSession* pConnection)
{
    LOGLESS_TRACE(logger);
    auto sessionIdIt = mConnectionToSessionId.find(pConnection);
    if (mConnectionToSessionId.end() == sessionIdIt)
    {
        return;
    }
    auto sessionId = sessionIdIt->second;
    mConnectionToSessionId.erase(sessionIdIt);
    auto sessionIt = mSessions.find(sessionId);
    sessionIt->second->connectionSession.reset();
}

void ProtocolHandler::onMsg(bfc::ConstBufferView pMsg, std::shared_ptr<IConnectionSession> pConnection)
{
    LOGLESS_TRACE(logger);
    PropertyTreeProtocol message;
    cum::per_codec_ctx context((std::byte*)pMsg.data(), pMsg.size());
    decode_per(message, context);

    std::string stred;
    str("root", message, stred, true);
    Logless(logger, "DBG ProtocolHandler: receive: session=%p; decoded=%s;", pConnection.get(),  stred.c_str());

    std::visit([this, &pConnection](auto&& pMsg) {
            onMsg(std::move(pMsg), pConnection);
        }, std::move(message));
}

void ProtocolHandler::onMsg(PropertyTreeMessage&& pMsg, std::shared_ptr<IConnectionSession>& pConnection)
{
    std::visit([this, transactionId = pMsg.transactionId, &pConnection](auto&& pMsg){
            handle(transactionId, std::move(pMsg), pConnection);
        }, std::move(pMsg.message));
}

void ProtocolHandler::onMsg(PropertyTreeMessageArray&& pMsg, std::shared_ptr<IConnectionSession>& pConnection)
{
    for (auto& i : pMsg)
    {
        onMsg(std::move(i), pConnection);
    }
}

void ProtocolHandler::handle(uint16_t pTransactionId, SigninRequest&& pMsg, std::shared_ptr<IConnectionSession>& pConnection)
{
    LOGLESS_TRACE(logger);
    PropertyTreeProtocol message = PropertyTreeMessage{};
    auto& propertyTreeMessage = std::get<PropertyTreeMessage>(message);
    propertyTreeMessage.transactionId = pTransactionId;
    propertyTreeMessage.message = SigninAccept{};

    auto sessionId = mSessionIdCtr++;
    mSessions.emplace(sessionId, std::make_shared<Session>(pConnection));
    mConnectionToSessionId.emplace(pConnection.get(), sessionId);

    send(message, pConnection);
}

void ProtocolHandler::handle(uint16_t pTransactionId, CreateRequest&& pMsg, std::shared_ptr<IConnectionSession>& pConnection)
{
    LOGLESS_TRACE(logger);
    PropertyTreeProtocol message = PropertyTreeMessage{};
    auto& propertyTreeMessage = std::get<PropertyTreeMessage>(message);
    propertyTreeMessage.transactionId = pTransactionId;
    propertyTreeMessage.message = CreateReject{};
    auto& createReject = std::get<CreateReject>(propertyTreeMessage.message);
    createReject.cause = Cause::NOT_FOUND;

    auto foundIt = mTree.find(pMsg.parentUuid);

    if (mTree.end() == foundIt)
    {
        send(message, pConnection);
        return;
    }
    auto node = foundIt->second;

    auto sessionIdIt = mConnectionToSessionId.find(pConnection.get());
    if (mConnectionToSessionId.end() == sessionIdIt)
    {
        Logless(logger, "ERR ProtocolHandler:: CreateRequest from a non signedin connection.");
        return;
    }
    auto sessionId = sessionIdIt->second;

    auto res = node->children.emplace(pMsg.name, std::make_shared<Node>(pMsg.name, sessionId, node, -1));
    auto insertedNode = res.first->second;

    if (false == res.second)
    {
        createReject.cause = Cause::ALREADY_EXIST;
        send(message, pConnection);
        return;
    }

    auto uuid = mUuidCtr++;
    insertedNode->uuid = uuid;

    mTree.emplace(uuid, insertedNode);

    propertyTreeMessage.message = CreateAccept{};
    auto& createAccept = std::get<CreateAccept>(propertyTreeMessage.message);
    createAccept.uuid = uuid;
    send(message, pConnection);

    {
        PropertyTreeProtocol message = PropertyTreeMessage{};
        auto& propertyTreeMessage = std::get<PropertyTreeMessage>(message);
        propertyTreeMessage.transactionId = 0xFFFF;
        propertyTreeMessage.message = TreeUpdateNotification{};
        auto& treeUpdateNotification = std::get<TreeUpdateNotification>(propertyTreeMessage.message);

        treeUpdateNotification.nodeToAddList.emplace_back(NamedNode{pMsg.name, insertedNode->uuid, node->uuid});

        std::byte buffer[ENCODE_SIZE];
        auto msgSize = encode(message, buffer, sizeof(buffer));

        for (auto& i : mSessions)
        {
            send(buffer, msgSize, i.second->connectionSession);
        }
    }
}

template <typename T>
void ProtocolHandler::fillToAddListFromTree(T& pIe, std::shared_ptr<Node>& pNode, bool pRecursive)
{
    LOGLESS_TRACE(logger);
    struct TraversalContext
    {
        TraversalContext(std::shared_ptr<Node> pNode, std::map<std::string, std::shared_ptr<Node>>::iterator pIt)
            : parentNode(pNode)
            , current(pIt)
        {}

        std::shared_ptr<Node> parentNode;
        std::map<std::string, std::shared_ptr<Node>>::iterator current;
    };

    auto currentNode = pNode;
    std::list<TraversalContext> levels;

    levels.emplace_back(TraversalContext(currentNode, currentNode->children.begin()));

    while (true)
    {
        auto& currentLevel = levels.back();

        if (currentLevel.parentNode->children.end() == currentLevel.current)
        {
            levels.pop_back();
            if (!levels.size())
            {
                break;
            }
            continue;
        }

        pIe.nodeToAddList.emplace_back(NamedNode{currentLevel.current->first, currentLevel.current->second->uuid, currentLevel.parentNode->uuid});
        if (currentLevel.current->second->children.size() && pRecursive)
        {
            levels.emplace_back(TraversalContext(currentLevel.current->second, currentLevel.current->second->children.begin()));
            currentLevel.current++;
            continue;
        }
        currentLevel.current++;
    }
}

void ProtocolHandler::handle(uint16_t pTransactionId, TreeInfoRequest&& pMsg, std::shared_ptr<IConnectionSession>& pConnection)
{
    LOGLESS_TRACE(logger);
    PropertyTreeProtocol message = PropertyTreeMessage{};
    auto& propertyTreeMessage = std::get<PropertyTreeMessage>(message);
    propertyTreeMessage.transactionId = pTransactionId;
    propertyTreeMessage.message = TreeInfoErrorResponse{};
    auto& treeInfoErrorResponse = std::get<TreeInfoErrorResponse>(propertyTreeMessage.message);
    treeInfoErrorResponse.cause = Cause::NOT_FOUND;

    auto foundItParent = mTree.find(pMsg.parentUuid);
    if (mTree.end() == foundItParent)
    {
        send(message, pConnection);
        return;
    }
    auto parentNode = foundItParent->second;

    std::shared_ptr<propertytree::Node> node;

    if ("." == pMsg.name)
    {
        node = parentNode;
    }
    else
    {
        auto foundIt = parentNode->children.find(pMsg.name);
        if (parentNode->children.end() == foundIt)
        {
            send(message, pConnection);
            return;
        }
        node = foundIt->second;
    }

    propertyTreeMessage.message = TreeInfoResponse{};
    auto& treeInfoResponse = std::get<TreeInfoResponse>(propertyTreeMessage.message);

    if ("." != pMsg.name)
    {
        treeInfoResponse.nodeToAddList.emplace_back(NamedNode{pMsg.name, node->uuid, parentNode->uuid});
    }

    fillToAddListFromTree(treeInfoResponse, node, pMsg.recursive);
    send(message, pConnection);
}

void ProtocolHandler::handle(uint16_t pTransactionId, SetValueRequest&& pMsg, std::shared_ptr<IConnectionSession>& pConnection)
{
    LOGLESS_TRACE(logger);
    if (0 == pMsg.uuid && 4 == pMsg.data.size())
    {
        uint32_t value;
        std::memcpy(&value, pMsg.data.data(), 4);
        if (9u == value)
        {
            Logless(logger, "INF ProtocolHandler: terminate signal received!");
            mTerminator();
            return;
        }
    }

    auto foundIt = mTree.find(pMsg.uuid);

    if (mTree.end() == foundIt)
    {
        return;
    }
    auto node = foundIt->second;

    node->data = std::move(pMsg.data);

    PropertyTreeProtocol message = PropertyTreeMessage{};
    auto& propertyTreeMessage = std::get<PropertyTreeMessage>(message);
    propertyTreeMessage.transactionId = pTransactionId;
    propertyTreeMessage.message = SetValueAccept{};
    send(message, pConnection);

    propertyTreeMessage.transactionId = 0xFFFF;
    propertyTreeMessage.message = UpdateNotification{};
    auto& updateNotification = std::get<UpdateNotification>(propertyTreeMessage.message);
    updateNotification.uuid = node->uuid;

    updateNotification.data = node->data;

    std::byte buffer[ENCODE_SIZE];
    auto msgSize = encode(message, buffer, sizeof(buffer));

    for (auto i = node->listener.begin(); node->listener.end() != i; i++)
    {
        auto connection = i->second.lock();
        if (!connection)
        {
            auto sessionIt = mSessions.find(i->first);
            if (mSessions.end() == sessionIt)
            {
                continue;
            }
            connection = sessionIt->second->connectionSession;
            if (!connection)
            {
                continue;
            }
            i->second = connection;
        }

        send(buffer, msgSize, connection);
    }
}

void ProtocolHandler::handle(uint16_t pTransactionId, GetRequest&& pMsg, std::shared_ptr<IConnectionSession>& pConnection)
{
    LOGLESS_TRACE(logger);
    PropertyTreeProtocol message = PropertyTreeMessage{};
    auto& propertyTreeMessage = std::get<PropertyTreeMessage>(message);
    propertyTreeMessage.transactionId = pTransactionId;
    propertyTreeMessage.message = GetReject{};
    auto& getReject = std::get<GetReject>(propertyTreeMessage.message);
    getReject.cause = Cause::NOT_FOUND;

    auto foundIt = mTree.find(pMsg.uuid);
    if (mTree.end() == foundIt)
    {
        send(message, pConnection);
        return;
    }
    auto node = foundIt->second;

    propertyTreeMessage.message = GetAccept{};
    auto& getAccept = std::get<GetAccept>(propertyTreeMessage.message);
    getAccept.data = node->data;

    send(message, pConnection);
}

void ProtocolHandler::handle(uint16_t pTransactionId, SubscribeRequest&& pMsg, std::shared_ptr<IConnectionSession>& pConnection)
{
    LOGLESS_TRACE(logger);
    PropertyTreeProtocol message = PropertyTreeMessage{};
    auto& propertyTreeMessage = std::get<PropertyTreeMessage>(message);
    propertyTreeMessage.transactionId = pTransactionId;
    propertyTreeMessage.message = SubscribeResponse{};
    auto& subscribeResponse = std::get<SubscribeResponse>(propertyTreeMessage.message);
    subscribeResponse.cause = Cause::NOT_FOUND;

    auto foundIt = mTree.find(pMsg.uuid);
    if (mTree.end() == foundIt)
    {
        send(message, pConnection);
        return;
    }
    auto node = foundIt->second;

    auto sessionIdIt = mConnectionToSessionId.find(pConnection.get());
    if (mConnectionToSessionId.end() == sessionIdIt)
    {
        Logless(logger, "ERR ProtocolHandler: SubscribeRequest from a non signedin connection.");
        return;
    }
    auto sessionId = sessionIdIt->second;

    node->listener[sessionId] = pConnection;

    subscribeResponse.cause = Cause::OK;
    send(message, pConnection);
}

void ProtocolHandler::handle(uint16_t pTransactionId, UnsubscribeRequest&& pMsg, std::shared_ptr<IConnectionSession>& pConnection)
{
    LOGLESS_TRACE(logger);
    PropertyTreeProtocol message = PropertyTreeMessage{};
    auto& propertyTreeMessage = std::get<PropertyTreeMessage>(message);
    propertyTreeMessage.transactionId = pTransactionId;
    propertyTreeMessage.message = UnsubscribeResponse{};
    auto& unsubscribeResponse = std::get<UnsubscribeResponse>(propertyTreeMessage.message);
    unsubscribeResponse.cause = Cause::NOT_FOUND;

    auto foundIt = mTree.find(pMsg.uuid);
    if (mTree.end() == foundIt)
    {
        send(message, pConnection);
        return;
    }
    auto node = foundIt->second;

    auto sessionIdIt = mConnectionToSessionId.find(pConnection.get());
    if (mConnectionToSessionId.end() == sessionIdIt)
    {
        return;
    }
    auto sessionId = sessionIdIt->second;

    auto listenerIt = node->listener.find(sessionId);
    if (node->listener.end() == listenerIt)
    {
        send(message, pConnection);
        return;
    }
    node->listener.erase(listenerIt);

    unsubscribeResponse.cause = Cause::OK;
    send(message, pConnection);
}

void ProtocolHandler::handle(uint16_t pTransactionId, DeleteRequest&& pMsg, std::shared_ptr<IConnectionSession>& pConnection)
{
    LOGLESS_TRACE(logger);

    PropertyTreeProtocol message = PropertyTreeMessage{};
    auto& propertyTreeMessage = std::get<PropertyTreeMessage>(message);
    propertyTreeMessage.transactionId = pTransactionId;
    propertyTreeMessage.message = DeleteResponse{};
    auto& deleteResponse = std::get<DeleteResponse>(propertyTreeMessage.message);
    deleteResponse.cause = Cause::NOT_FOUND;

    auto foundIt = mTree.find(pMsg.uuid);
    if (mTree.end() == foundIt)
    {
        send(message, pConnection);
        return;
    }
    auto node = foundIt->second;

    if (node->children.size())
    {
        deleteResponse.cause = Cause::NOT_EMPTY;
        send(message, pConnection);
        return;
    }
    auto name = node->name;

    auto parentNode = node->parent.lock();
    mTree.erase(pMsg.uuid);

    parentNode->children.erase(name);

    deleteResponse.cause = Cause::OK;
    send(message, pConnection);

    {
        PropertyTreeProtocol message = PropertyTreeMessage{};
        auto& propertyTreeMessage = std::get<PropertyTreeMessage>(message);
        propertyTreeMessage.transactionId = 0xFFFF;
        propertyTreeMessage.message = TreeUpdateNotification{};
        auto& treeUpdateNotification = std::get<TreeUpdateNotification>(propertyTreeMessage.message);

        treeUpdateNotification.nodeToDelete.emplace_back(pMsg.uuid);

        std::byte buffer[ENCODE_SIZE];
        auto msgSize = encode(message, buffer, sizeof(buffer));

        for (auto& i : mSessions)
        {
            send(buffer, msgSize, i.second->connectionSession);
        }
    }
}

void ProtocolHandler::handle(uint16_t pTransactionId, RpcRequest&& pMsg, std::shared_ptr<IConnectionSession>& pConnection)
{
    LOGLESS_TRACE(logger);
    PropertyTreeProtocol message = PropertyTreeMessage{};
    auto& propertyTreeMessage = std::get<PropertyTreeMessage>(message);
    propertyTreeMessage.message = std::move(pMsg);

    PropertyTreeProtocol messageReject = PropertyTreeMessage{};
    auto& propertyTreeMessageReject = std::get<PropertyTreeMessage>(messageReject);
    propertyTreeMessageReject.message = RpcReject{};
    propertyTreeMessageReject.transactionId = pTransactionId;
    auto& rpcReject = std::get<RpcReject>(propertyTreeMessageReject.message);

    auto foundIt = mTree.find(pMsg.uuid);
    if (mTree.end() == foundIt)
    {
        rpcReject.cause = Cause::NOT_FOUND;
        send(messageReject, pConnection);
        return;
    }
    auto node = foundIt->second;

    auto sourceSessionIt = mConnectionToSessionId.find(pConnection.get());
    if (mConnectionToSessionId.end() == sourceSessionIt)
    {
        return;
    }
    auto sourceSessionId = sourceSessionIt->second;
    auto sessionId = node->sessionId;
    auto targetConnection =  mSessions.find(sessionId)->second->connectionSession;

    if (!targetConnection)
    {
        return;
    }

    auto trId = mTrIdCtr++;
    propertyTreeMessage.transactionId = trId;

    mTrIdTranslation[trId] = std::pair<uint32_t, uint16_t>(sourceSessionId, pTransactionId);

    send(message, targetConnection);
}

template<typename T>
void ProtocolHandler::handleRpc(uint16_t pTransactionId, T&& pMsg, std::shared_ptr<IConnectionSession>& pConnection)
{
    LOGLESS_TRACE(logger);
    auto translationIt = mTrIdTranslation.find(pTransactionId);
    if (mTrIdTranslation.end() == translationIt)
    {
        return;
    }
    auto translation = translationIt->second;
    mTrIdTranslation.erase(translationIt);

    PropertyTreeProtocol message = PropertyTreeMessage{};
    auto& propertyTreeMessage = std::get<PropertyTreeMessage>(message);
    propertyTreeMessage.message = std::move(pMsg);
    propertyTreeMessage.transactionId = translation.second;

    auto targetSessionIt =  mSessions.find(translation.first);
    if (mSessions.end() == targetSessionIt)
    {
        return;
    }
    auto targetSession = targetSessionIt->second;
    send(message, targetSession->connectionSession);
}

void ProtocolHandler::handle(uint16_t pTransactionId, RpcAccept&& pMsg, std::shared_ptr<IConnectionSession>& pConnection)
{
    handleRpc(pTransactionId, std::move(pMsg), pConnection);
}

void ProtocolHandler::handle(uint16_t pTransactionId, RpcReject&& pMsg, std::shared_ptr<IConnectionSession>& pConnection)
{
    handleRpc(pTransactionId, std::move(pMsg), pConnection);
}

void ProtocolHandler::handle(uint16_t pTransactionId, HearbeatRequest&& pMsg, std::shared_ptr<IConnectionSession>& pConnection)
{
    LOGLESS_TRACE(logger);
    PropertyTreeProtocol message = PropertyTreeMessage{};
    auto& propertyTreeMessage = std::get<PropertyTreeMessage>(message);
    propertyTreeMessage.message = HearbeatResponse{};
    propertyTreeMessage.transactionId = pTransactionId;
    send(message, pConnection);
}

size_t ProtocolHandler::encode(const PropertyTreeProtocol& pMsg, std::byte* pData, size_t pSize)
{
    LOGLESS_TRACE(logger);
    auto& msgSize = *(new (pData) uint16_t(0));
    cum::per_codec_ctx context(pData+sizeof(msgSize), pSize-sizeof(msgSize));
    encode_per(pMsg, context);
    msgSize = pSize-context.size()-2;

    std::string stred;
    str("root", pMsg, stred, true);
    Logless(logger, "DBG ProtocolHandler: send: encoded=%s;", stred.c_str());

    return msgSize + 2;
}

void ProtocolHandler::send(const PropertyTreeProtocol& pMsg, std::shared_ptr<IConnectionSession>& pConnection)
{
    if (!pConnection)
    {
        return;
    }
    LOGLESS_TRACE(logger);

    std::byte buffer[ENCODE_SIZE];
    auto msgSize = encode(pMsg, buffer, sizeof(buffer));

    Logless(logger, "DBG ProtocolHandler: send: session=%p;", pConnection.get());
    pConnection->send(bfc::ConstBufferView(buffer, msgSize));
 }

void ProtocolHandler::send(const std::byte* pData, size_t pSize, std::shared_ptr<IConnectionSession>& pConnection)
{
    if (!pConnection)
    {
        return;
    }
    LOGLESS_TRACE(logger);
    Logless(logger, "DBG ProtocolHandler: send: session=%p;", pConnection.get());
    pConnection->send(bfc::ConstBufferView(pData, pSize));
}


} // propertytree
