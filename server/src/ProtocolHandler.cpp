#include <list>

#include <bfc/ThreadPool.hpp>
#include <bfc/Timer.hpp>
#include <bfc/Singleton.hpp>
#include <bfc/Buffer.hpp>

#include <logless/Logger.hpp>

#include <interface/protocol.hpp>

#include <IConnectionSession.hpp>

#include <ProtocolHandler.hpp>

namespace propertytree
{

constexpr size_t ENCODE_SIZE = 1024*16;

ProtocolHandler::ProtocolHandler(bfc::LightFn<void()> pTerminator)
    : mTp(bfc::Singleton<bfc::ThreadPool<>>::get())
    , mTimer(bfc::Singleton<bfc::Timer<>>::get())
    , mTerminator(pTerminator)
{
    auto rootUUid = mUuidCtr.fetch_add(1);

    std::unique_lock<std::mutex> lg(mTreeMutex);
    mTree.emplace(rootUUid, std::make_shared<Node>("", 0xFFFFFFFF, std::weak_ptr<Node>(), rootUUid));
}

void ProtocolHandler::onDisconnect(IConnectionSession* pConnection)
{
    std::unique_lock<std::mutex> lgSession(mSessionsMutex);
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

void ProtocolHandler::onMsg(bfc::ConstBuffer pMsg, std::shared_ptr<IConnectionSession> pConnection)
{
    // TODO: this is a workaround to deal with function object that needs to be copyable
    struct lambda
    {
        lambda(ProtocolHandler* pSelf, bfc::ConstBuffer&& pMsg, std::shared_ptr<IConnectionSession>&& pConnection)
            : self(pSelf)
            , pMsg(std::move(pMsg))
            , pConnection(std::move(pConnection))
        {}

        ProtocolHandler* self;
        bfc::ConstBuffer pMsg;
        std::shared_ptr<IConnectionSession> pConnection;

        void run()
        {
            PropertyTreeProtocol message;
            cum::per_codec_ctx context((std::byte*)pMsg.data(), pMsg.size());
            decode_per(message, context);

            std::string stred;
            str("root", message, stred, true);
            Logless("DBG ProtocolHandler: receive: session=_ decoded=_", pConnection.get(),  stred.c_str());

            std::visit([this](auto&& pMsg) {
                    this->self->onMsg(std::move(pMsg), this->pConnection);
                }, std::move(message));
        }
    };

    auto instance = new lambda(this, std::move(pMsg), std::move(pConnection));
    mTp.execute([instance]() mutable {
            instance->run();
            delete instance;
        });
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
    PropertyTreeProtocol message = PropertyTreeMessage{};
    auto& propertyTreeMessage = std::get<PropertyTreeMessage>(message);
    propertyTreeMessage.transactionId = pTransactionId;
    propertyTreeMessage.message = SigninAccept{};

    auto sessionId = mSessionIdCtr.fetch_add(1);
    std::unique_lock<std::mutex> lgSession(mSessionsMutex);
    mSessions.emplace(sessionId, std::make_shared<Session>(pConnection));
    mConnectionToSessionId.emplace(pConnection.get(), sessionId);
    lgSession.unlock();

    send(message, pConnection);
}

void ProtocolHandler::handle(uint16_t pTransactionId, CreateRequest&& pMsg, std::shared_ptr<IConnectionSession>& pConnection)
{
    PropertyTreeProtocol message = PropertyTreeMessage{};
    auto& propertyTreeMessage = std::get<PropertyTreeMessage>(message);
    propertyTreeMessage.transactionId = pTransactionId;
    propertyTreeMessage.message = CreateReject{};
    auto& createReject = std::get<CreateReject>(propertyTreeMessage.message);
    createReject.cause = Cause::NOT_FOUND;

    std::unique_lock<std::mutex> lgTree(mTreeMutex);
    auto foundIt = mTree.find(pMsg.parentUuid);

    if (mTree.end() == foundIt)
    {
        send(message, pConnection);
        return;
    }
    auto node = foundIt->second;
    lgTree.unlock();

    std::unique_lock<std::mutex> lgSession(mSessionsMutex);
    auto sessionIdIt = mConnectionToSessionId.find(pConnection.get());
    if (mConnectionToSessionId.end() == sessionIdIt)
    {
        Logless("ERR ProtocolHandler:: CreateRequest from a non signedin connection.");
        return;
    }
    auto sessionId = sessionIdIt->second;
    lgSession.unlock();

    std::unique_lock<std::mutex> lgChildren(node->childrenMutex);
    auto res = node->children.emplace(pMsg.name, std::make_shared<Node>(pMsg.name, sessionId, node, -1));
    auto insertedNode = res.first->second;
    lgChildren.unlock();

    if (false == res.second)
    {
        createReject.cause = Cause::ALREADY_EXIST;
        send(message, pConnection);
        return;
    }

    auto uuid = mUuidCtr.fetch_add(1);
    insertedNode->uuid = uuid;

    lgTree.lock();
    mTree.emplace(uuid, insertedNode);
    lgTree.unlock();

    propertyTreeMessage.message = CreateAccept{};
    auto& createAccept = std::get<CreateAccept>(propertyTreeMessage.message);
    createAccept.uuid = uuid;
    send(message, pConnection);

    lgSession.lock();
    {
        PropertyTreeProtocol message = PropertyTreeMessage{};
        auto& propertyTreeMessage = std::get<PropertyTreeMessage>(message);
        propertyTreeMessage.transactionId = 0xFFFF;
        propertyTreeMessage.message = TreeUpdateNotification{};
        auto& treeUpdateNotification = std::get<TreeUpdateNotification>(propertyTreeMessage.message);

        treeUpdateNotification.nodeToAddList.emplace_back(NamedNode{pMsg.name, insertedNode->uuid, node->uuid});

        std::byte buffer[ENCODE_SIZE];
        auto msgSize = encode(message, buffer, sizeof(buffer));

        // TODO: this blocks mSessionsMutex too much
        for (auto& i : mSessions)
        {
            send(buffer, msgSize, i.second->connectionSession);
        }
    }
}

template <typename T>
void ProtocolHandler::fillToAddListFromTree(T& pIe, std::shared_ptr<Node>& pNode, bool pRecursive)
{
    struct TraversalContext
    {
        TraversalContext(std::shared_ptr<Node>& pNode)
            : parentNode(pNode)
        {
            std::unique_lock<std::mutex> lg(pNode->childrenMutex);
            current = pNode->children.begin()->first;
        }

        std::shared_ptr<Node> parentNode;
        std::string current;
    };

    auto currentNode = pNode;

    std::unique_lock<std::mutex> lgChildren(pNode->childrenMutex);
    if (!currentNode->children.size())
    {
        return;
    }
    lgChildren.unlock();

    std::list<TraversalContext> levels;

    levels.emplace_back(TraversalContext(currentNode));

    while (true)
    {
        auto currentLevel = &levels.back();
        auto& parentNode = currentLevel->parentNode;

        std::unique_lock<std::mutex> lgChildren(parentNode->childrenMutex);
        auto currentIt = parentNode->children.equal_range(currentLevel->current).first;

        if (currentLevel->parentNode->children.end() != currentIt)
        {
            Logless("DGB ProtocolHandler: Tree traversal: level:_ uuid:_ \"_/_\"", levels.size(), currentIt->second->uuid, parentNode->name.c_str(), currentIt->first.c_str());

            pIe.nodeToAddList.emplace_back(NamedNode{currentIt->first, currentIt->second->uuid, parentNode->uuid});
            if (currentIt->second->children.size() && pRecursive)
            {
                levels.emplace_back(TraversalContext(currentIt->second));
                continue;
            }

            currentIt++;
        }
        if (currentLevel->parentNode->children.end() == currentIt)
        {
            bool mainLoopBreak = false;
            while (currentLevel->parentNode->children.end() == currentIt)
            {
                lgChildren = {};
                levels.pop_back();
                if (!levels.size())
                {
                    mainLoopBreak = true;
                    break;
                }

                currentLevel = &levels.back();
                std::unique_lock<std::mutex> lgChildren(currentLevel->parentNode->childrenMutex);
                auto nextIts = currentLevel->parentNode->children.equal_range(currentLevel->current);
                currentIt = nextIts.second;
                currentLevel->current = currentIt->first; 
            }

            if (mainLoopBreak)
            {
                break;
            }
        }
        else
        {
            auto nextIts = parentNode->children.equal_range(currentLevel->current);
            currentLevel->current = nextIts.second->first; 
        }
    }
}

void ProtocolHandler::handle(uint16_t pTransactionId, TreeInfoRequest&& pMsg, std::shared_ptr<IConnectionSession>& pConnection)
{
    PropertyTreeProtocol message = PropertyTreeMessage{};
    auto& propertyTreeMessage = std::get<PropertyTreeMessage>(message);
    propertyTreeMessage.transactionId = pTransactionId;
    propertyTreeMessage.message = TreeInfoErrorResponse{};
    auto& treeInfoErrorResponse = std::get<TreeInfoErrorResponse>(propertyTreeMessage.message);
    treeInfoErrorResponse.cause = Cause::NOT_FOUND;

    std::unique_lock<std::mutex> lgTree(mTreeMutex);
    auto foundItParent = mTree.find(pMsg.parentUuid);
    if (mTree.end() == foundItParent)
    {
        send(message, pConnection);
        return;
    }
    auto parentNode = foundItParent->second;
    lgTree.unlock();

    std::shared_ptr<propertytree::Node> node;

    if ("." == pMsg.name)
    {
        node = parentNode;
    }
    else
    {
        std::unique_lock<std::mutex> lgChildren(parentNode->childrenMutex);
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
    if (0 == pMsg.uuid && 4 == pMsg.data.size())
    {
        uint32_t value;
        std::memcpy(&value, pMsg.data.data(), 4);
        if (9u == value)
        {
            Logless("INF ProtocolHandler: terminate signal received!");
            mTerminator();
            return;
        }
    }

    std::unique_lock<std::mutex> lgTree(mTreeMutex);
    auto foundIt = mTree.find(pMsg.uuid);

    if (mTree.end() == foundIt)
    {
        return;
    }
    auto node = foundIt->second;
    lgTree.unlock();

    std::unique_lock<std::mutex> lgData(node->dataMutex);
    node->data = std::move(pMsg.data);
    lgData.unlock();

    PropertyTreeProtocol message = PropertyTreeMessage{};
    auto& propertyTreeMessage = std::get<PropertyTreeMessage>(message);
    propertyTreeMessage.transactionId = pTransactionId;
    propertyTreeMessage.message = SetValueAccept{};
    send(message, pConnection);

    propertyTreeMessage.transactionId = 0xFFFF;
    propertyTreeMessage.message = UpdateNotification{};
    auto& updateNotification = std::get<UpdateNotification>(propertyTreeMessage.message);
    updateNotification.uuid = node->uuid;

    lgData.lock();
    updateNotification.data = node->data;
    lgData.unlock();

    std::byte buffer[ENCODE_SIZE];
    auto msgSize = encode(message, buffer, sizeof(buffer));

    std::unique_lock<std::mutex> lgListener(node->listenerMutex);
    for (auto i = node->listener.begin(); node->listener.end() != i; i++)
    {
        auto connection = i->second.lock();
        if (!connection)
        {
            std::unique_lock<std::mutex> lgSession(mSessionsMutex);
            auto sessionIt = mSessions.find(i->first);
            if (mSessions.end() == sessionIt)
            {
                // TODO: removal of deleted session on the listener list
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
    PropertyTreeProtocol message = PropertyTreeMessage{};
    auto& propertyTreeMessage = std::get<PropertyTreeMessage>(message);
    propertyTreeMessage.transactionId = pTransactionId;
    propertyTreeMessage.message = GetReject{};
    auto& getReject = std::get<GetReject>(propertyTreeMessage.message);
    getReject.cause = Cause::NOT_FOUND;

    std::unique_lock<std::mutex> lgTree(mTreeMutex);
    auto foundIt = mTree.find(pMsg.uuid);
    if (mTree.end() == foundIt)
    {
        send(message, pConnection);
        return;
    }
    auto node = foundIt->second;
    lgTree.unlock();

    std::unique_lock<std::mutex> lgData(node->dataMutex);
    propertyTreeMessage.message = GetAccept{};
    auto& getAccept = std::get<GetAccept>(propertyTreeMessage.message);
    getAccept.data = node->data;
    lgData.unlock();

    send(message, pConnection);
}

void ProtocolHandler::handle(uint16_t pTransactionId, SubscribeRequest&& pMsg, std::shared_ptr<IConnectionSession>& pConnection)
{
    PropertyTreeProtocol message = PropertyTreeMessage{};
    auto& propertyTreeMessage = std::get<PropertyTreeMessage>(message);
    propertyTreeMessage.transactionId = pTransactionId;
    propertyTreeMessage.message = SubscribeResponse{};
    auto& subscribeResponse = std::get<SubscribeResponse>(propertyTreeMessage.message);
    subscribeResponse.cause = Cause::NOT_FOUND;

    std::unique_lock<std::mutex> lgTree(mTreeMutex);
    auto foundIt = mTree.find(pMsg.uuid);
    if (mTree.end() == foundIt)
    {
        send(message, pConnection);
        return;
    }
    auto node = foundIt->second;
    lgTree.unlock();

    std::unique_lock<std::mutex> lgSession(mSessionsMutex);
    auto sessionIdIt = mConnectionToSessionId.find(pConnection.get());
    if (mConnectionToSessionId.end() == sessionIdIt)
    {
        Logless("ERR ProtocolHandler: SubscribeRequest from a non signedin connection.");
        return;
    }
    auto sessionId = sessionIdIt->second;
    lgSession.unlock();

    std::unique_lock<std::mutex> lgListener(node->listenerMutex);
    node->listener[sessionId] = pConnection;
    lgListener.unlock();

    subscribeResponse.cause = Cause::OK;
    send(message, pConnection);
}

void ProtocolHandler::handle(uint16_t pTransactionId, UnsubscribeRequest&& pMsg, std::shared_ptr<IConnectionSession>& pConnection)
{
    PropertyTreeProtocol message = PropertyTreeMessage{};
    auto& propertyTreeMessage = std::get<PropertyTreeMessage>(message);
    propertyTreeMessage.transactionId = pTransactionId;
    propertyTreeMessage.message = UnsubscribeResponse{};
    auto& unsubscribeResponse = std::get<UnsubscribeResponse>(propertyTreeMessage.message);
    unsubscribeResponse.cause = Cause::NOT_FOUND;

    std::unique_lock<std::mutex> lgTree(mTreeMutex);
    auto foundIt = mTree.find(pMsg.uuid);
    if (mTree.end() == foundIt)
    {
        send(message, pConnection);
        return;
    }
    auto node = foundIt->second;
    lgTree.unlock();

    std::unique_lock<std::mutex> lgSession(mSessionsMutex);
    auto sessionIdIt = mConnectionToSessionId.find(pConnection.get());
    if (mConnectionToSessionId.end() == sessionIdIt)
    {
        return;
    }
    auto sessionId = sessionIdIt->second;
    lgSession.unlock();

    std::unique_lock<std::mutex> lgListener(node->listenerMutex);
    auto listenerIt = node->listener.find(sessionId);
    if (node->listener.end() == listenerIt)
    {
        send(message, pConnection);
        return;
    }
    node->listener.erase(listenerIt);
    lgListener.unlock();

    unsubscribeResponse.cause = Cause::OK;
    send(message, pConnection);
}

void ProtocolHandler::handle(uint16_t pTransactionId, DeleteRequest&& pMsg, std::shared_ptr<IConnectionSession>& pConnection)
{
    // TODO: blocks deletion of uuid=0

    PropertyTreeProtocol message = PropertyTreeMessage{};
    auto& propertyTreeMessage = std::get<PropertyTreeMessage>(message);
    propertyTreeMessage.transactionId = pTransactionId;
    propertyTreeMessage.message = DeleteResponse{};
    auto& deleteResponse = std::get<DeleteResponse>(propertyTreeMessage.message);
    deleteResponse.cause = Cause::NOT_FOUND;

    std::unique_lock<std::mutex> lgTree(mTreeMutex);
    auto foundIt = mTree.find(pMsg.uuid);
    if (mTree.end() == foundIt)
    {
        send(message, pConnection);
        return;
    }
    auto node = foundIt->second;
    lgTree.unlock();

    std::unique_lock<std::mutex> lgChildren(node->childrenMutex);
    if (node->children.size())
    {
        deleteResponse.cause = Cause::NOT_EMPTY;
        send(message, pConnection);
        return;
    }
    auto parentNode = node->parent.lock();
    auto name = node->name;
    lgChildren.unlock();

    lgTree.lock();
    mTree.erase(pMsg.uuid);
    lgTree.unlock();

    std::unique_lock<std::mutex> lgParentChildren(parentNode->childrenMutex);
    parentNode->children.erase(name);
    lgParentChildren.unlock();

    deleteResponse.cause = Cause::OK;
    send(message, pConnection);

    std::unique_lock<std::mutex> lgSession(mSessionsMutex);
    {
        PropertyTreeProtocol message = PropertyTreeMessage{};
        auto& propertyTreeMessage = std::get<PropertyTreeMessage>(message);
        propertyTreeMessage.transactionId = 0xFFFF;
        propertyTreeMessage.message = TreeUpdateNotification{};
        auto& treeUpdateNotification = std::get<TreeUpdateNotification>(propertyTreeMessage.message);

        treeUpdateNotification.nodeToDelete.emplace_back(pMsg.uuid);

        std::byte buffer[ENCODE_SIZE];
        auto msgSize = encode(message, buffer, sizeof(buffer));

        // TODO: this blocks mSessionsMutex too much
        for (auto& i : mSessions)
        {
            send(buffer, msgSize, i.second->connectionSession);
        }
    }
}

void ProtocolHandler::handle(uint16_t pTransactionId, RpcRequest&& pMsg, std::shared_ptr<IConnectionSession>& pConnection)
{
    PropertyTreeProtocol message = PropertyTreeMessage{};
    auto& propertyTreeMessage = std::get<PropertyTreeMessage>(message);
    propertyTreeMessage.message = std::move(pMsg);

    PropertyTreeProtocol messageReject = PropertyTreeMessage{};
    auto& propertyTreeMessageReject = std::get<PropertyTreeMessage>(messageReject);
    propertyTreeMessageReject.message = RpcReject{};
    propertyTreeMessageReject.transactionId = pTransactionId;
    auto& rpcReject = std::get<RpcReject>(propertyTreeMessageReject.message);

    std::unique_lock<std::mutex> lgTree(mTreeMutex);
    auto foundIt = mTree.find(pMsg.uuid);
    if (mTree.end() == foundIt)
    {
        rpcReject.cause = Cause::NOT_FOUND;
        send(messageReject, pConnection);
        return;
    }
    auto node = foundIt->second;
    lgTree.unlock();

    std::unique_lock<std::mutex> lgSession(mSessionsMutex);
    auto sourceSessionIt = mConnectionToSessionId.find(pConnection.get());
    if (mConnectionToSessionId.end() == sourceSessionIt)
    {
        return;
    }
    auto sourceSessionId = sourceSessionIt->second;
    auto sessionId = node->sessionId;
    auto targetConnection =  mSessions.find(sessionId)->second->connectionSession;
    lgSession.unlock();

    if (!targetConnection)
    {
        return;
    }

    auto trId = mTrIdCtr.fetch_add(1);
    propertyTreeMessage.transactionId = trId;

    std::unique_lock<std::mutex> lgTrId(mTrIdTranslationMutex);
    mTrIdTranslation[trId] = std::pair<uint32_t, uint16_t>(sourceSessionId, pTransactionId);
    lgTrId.unlock();

    send(message, targetConnection);
}

template<typename T>
void ProtocolHandler::handleRpc(uint16_t pTransactionId, T&& pMsg, std::shared_ptr<IConnectionSession>& pConnection)
{
    std::unique_lock<std::mutex> lgTrId(mTrIdTranslationMutex);
    auto translationIt = mTrIdTranslation.find(pTransactionId);
    if (mTrIdTranslation.end() == translationIt)
    {
        return;
    }
    auto translation = translationIt->second;
    mTrIdTranslation.erase(translationIt);
    lgTrId.unlock();

    PropertyTreeProtocol message = PropertyTreeMessage{};
    auto& propertyTreeMessage = std::get<PropertyTreeMessage>(message);
    propertyTreeMessage.message = std::move(pMsg);
    propertyTreeMessage.transactionId = translation.second;

    std::unique_lock<std::mutex> lgSession(mSessionsMutex);
    auto targetSessionIt =  mSessions.find(translation.first);
    if (mSessions.end() == targetSessionIt)
    {
        return;
    }
    auto targetSession = targetSessionIt->second;
    lgSession.unlock();
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

size_t ProtocolHandler::encode(const PropertyTreeProtocol& pMsg, std::byte* pData, size_t pSize)
{
    auto& msgSize = *(new (pData) uint16_t(0));
    cum::per_codec_ctx context(pData+sizeof(msgSize), pSize-sizeof(msgSize));
    encode_per(pMsg, context);
    msgSize = pSize-context.size()-2;

    std::string stred;
    str("root", pMsg, stred, true);
    Logless("DBG ProtocolHandler: send: encoded=_", stred.c_str());

    return msgSize + 2;
}

void ProtocolHandler::send(const PropertyTreeProtocol& pMsg, std::shared_ptr<IConnectionSession>& pConnection)
{
    if (!pConnection)
    {
        return;
    }

    std::byte buffer[ENCODE_SIZE];
    auto msgSize = encode(pMsg, buffer, sizeof(buffer));

    Logless("DBG ProtocolHandler: send: session=_", pConnection.get());
    pConnection->send(bfc::ConstBufferView(buffer, msgSize));
 }

void ProtocolHandler::send(const std::byte* pData, size_t pSize, std::shared_ptr<IConnectionSession>& pConnection)
{
    if (!pConnection)
    {
        return;
    }

    Logless("DBG ProtocolHandler: send: session=_", pConnection.get());
    pConnection->send(bfc::ConstBufferView(pData, pSize));
}


} // propertytree
