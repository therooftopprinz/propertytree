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

ProtocolHandler::ProtocolHandler()
    : mTp(bfc::Singleton<bfc::ThreadPool<>>::get())
    , mTimer(bfc::Singleton<bfc::Timer<>>::get())
{
    std::unique_lock<std::mutex> lg(mTreeMutex);
    auto rootUUid = mUuidCtr.fetch_add(1);
    mTree.emplace(rootUUid, std::make_shared<Node>(std::weak_ptr<Node>(), rootUUid));
}

void ProtocolHandler::onDisconnect(IConnectionSession* pConnection)
{
    std::unique_lock<std::mutex> lg(mSessionsMutex);
    auto sessionIdIt = mConnectionToSessionId.find(pConnection);
    auto sessionId = sessionIdIt->second;
    mConnectionToSessionId.erase(sessionIdIt);
    auto sessionIt = mSessions.find(sessionId);
    sessionIt->second.connectionSession.reset();
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
            Logless("ProtocolHandler: receive: session=_ decoded=_", pConnection.get(),  stred.c_str());

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
    std::unique_lock<std::mutex> lg(mSessionsMutex);
    mSessions.emplace(sessionId, Session{pConnection});
    mConnectionToSessionId.emplace(pConnection.get(), sessionId);
    lg.unlock();

    send(message, pConnection);
}

void ProtocolHandler::handle(uint16_t pTransactionId, CreateRequest&& pMsg, std::shared_ptr<IConnectionSession>& pConnection)
{
    std::unique_lock<std::mutex> lg(mTreeMutex);
    auto foundIt = mTree.find(pMsg.parentUuid);

    PropertyTreeProtocol message = PropertyTreeMessage{};
    auto& propertyTreeMessage = std::get<PropertyTreeMessage>(message);
    propertyTreeMessage.transactionId = pTransactionId;
    propertyTreeMessage.message = CreateReject{};
    auto& createReject = std::get<CreateReject>(propertyTreeMessage.message);
    createReject.cause = Cause::NOT_FOUND;

    if (mTree.end() == foundIt)
    {
        send(message, pConnection);
        return;
    }
    auto node = foundIt->second;
    lg.unlock();

    std::unique_lock<std::mutex> lgNode(node->mutex);
    auto res = node->children.emplace(pMsg.name, std::make_shared<Node>(node, -1));
    auto insertedNode = res.first->second;
    lgNode.unlock();

    if (false == res.second)
    {
        createReject.cause = Cause::ALREADY_EXIST;
        send(message, pConnection);
        return;
    }

    auto uuid = mUuidCtr.fetch_add(1);
    insertedNode->uuid = uuid;
    mTree.emplace(uuid, insertedNode);

    propertyTreeMessage.message = CreateAccept{};
    auto& createAccept = std::get<CreateAccept>(propertyTreeMessage.message);
    createAccept.uuid = uuid;
    send(message, pConnection);

    std::unique_lock<std::mutex> lgSession(mSessionsMutex);
    {
        PropertyTreeProtocol message = PropertyTreeMessage{};
        auto& propertyTreeMessage = std::get<PropertyTreeMessage>(message);
        propertyTreeMessage.transactionId = 0xFFFF;
        propertyTreeMessage.message = TreeUpdateNotification{};
        auto& treeUpdateNotification = std::get<TreeUpdateNotification>(propertyTreeMessage.message);

        treeUpdateNotification.nodeToAddList.emplace_back(NamedNode{pMsg.name, insertedNode->uuid, node->uuid});

        for (auto& i : mSessions)
        {
            send(message, i.second.connectionSession);
        }
    }
}

template <typename T>
void ProtocolHandler::fillToAddListFromTree(T& pIe, std::shared_ptr<Node>& pNode)
{
    struct TraversalContext
    {
        TraversalContext(std::shared_ptr<Node> pNode, std::map<std::string, std::shared_ptr<Node>>::iterator pIt)
            : lg(pNode->mutex)
            , parentNode(pNode)
            , current(pIt)
        {}

        std::unique_lock<std::mutex> lg;
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
        if (currentLevel.current->second->children.size())
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
    std::unique_lock<std::mutex> lg(mTreeMutex);
    auto foundItParent = mTree.find(pMsg.parentUuid);

    PropertyTreeProtocol message = PropertyTreeMessage{};
    auto& propertyTreeMessage = std::get<PropertyTreeMessage>(message);
    propertyTreeMessage.transactionId = pTransactionId;
    propertyTreeMessage.message = TreeInfoErrorResponse{};
    auto& treeInfoErrorResponse = std::get<TreeInfoErrorResponse>(propertyTreeMessage.message);
    treeInfoErrorResponse.cause = Cause::NOT_FOUND;

    if (mTree.end() == foundItParent)
    {
        send(message, pConnection);
        return;
    }

    auto parentNode = foundItParent->second;
    lg.unlock();

    std::unique_lock<std::mutex> lgNode(parentNode->mutex);
    auto foundIt = parentNode->children.find(pMsg.name);

    if (parentNode->children.end() == foundIt)
    {
        send(message, pConnection);
        return;
    }

    auto node = foundIt->second;
    lgNode.unlock();

    propertyTreeMessage.message = TreeInfoResponse{};
    auto& treeInfoResponse = std::get<TreeInfoResponse>(propertyTreeMessage.message);
    treeInfoResponse.nodeToAddList.emplace_back(NamedNode{pMsg.name, node->uuid, parentNode->uuid});

    fillToAddListFromTree(treeInfoResponse, node);

    send(message, pConnection);
}

void ProtocolHandler::handle(uint16_t pTransactionId, SetValueIndication&& pMsg, std::shared_ptr<IConnectionSession>& pConnection)
{
    std::unique_lock<std::mutex> lg(mTreeMutex);
    auto foundIt = mTree.find(pMsg.uuid);

    if (mTree.end() == foundIt)
    {
        return;
    }
    auto node = foundIt->second;
    lg.unlock();

    std::unique_lock<std::mutex> lgNode(node->mutex);

    if (node->data.size() == pMsg.data.size())
    {
        std::memcpy(node->data.data(), pMsg.data.data(), node->data.size());
    }
    else
    {
        auto buff = new std::byte[pMsg.data.size()];
        std::memcpy(buff, pMsg.data.data(), pMsg.data.size());
        node->data = bfc::Buffer(buff, pMsg.data.size());
    }
}

void ProtocolHandler::handle(uint16_t pTransactionId, GetRequest&& pMsg, std::shared_ptr<IConnectionSession>& pConnection)
{
    std::unique_lock<std::mutex> lg(mTreeMutex);
    auto foundIt = mTree.find(pMsg.uuid);

    PropertyTreeProtocol message = PropertyTreeMessage{};
    auto& propertyTreeMessage = std::get<PropertyTreeMessage>(message);
    propertyTreeMessage.transactionId = pTransactionId;
    propertyTreeMessage.message = GetReject{};
    auto& getReject = std::get<GetReject>(propertyTreeMessage.message);
    getReject.cause = Cause::NOT_FOUND;

    if (mTree.end() == foundIt)
    {
        send(message, pConnection);
        return;
    }
    auto node = foundIt->second;
    lg.unlock();

    std::unique_lock<std::mutex> lgNode(node->mutex);

    propertyTreeMessage.message = GetAccept{};
    auto& getAccept = std::get<GetAccept>(propertyTreeMessage.message);
    for (auto i=0u; i<node->data.size(); i++)
    {
        getAccept.data.emplace_back((uint8_t)node->data.data()[i]);
    }
    send(message, pConnection);
}

 void ProtocolHandler::send(const PropertyTreeProtocol& pMsg, std::shared_ptr<IConnectionSession>& pConnection)
 {
    std::byte buffer[512];
    auto& msgSize = *(new (buffer) uint16_t(0));
    cum::per_codec_ctx context(buffer+sizeof(msgSize), sizeof(buffer)-sizeof(msgSize));
    encode_per(pMsg, context);

    std::string stred;
    str("root", pMsg, stred, true);
    Logless("ProtocolHandler: send: session=_ encoded=_", pConnection.get(), stred.c_str());

    if (pConnection)
    {
        pConnection->send(bfc::ConstBufferView(buffer, msgSize+2));
    }
 }

} // propertytree