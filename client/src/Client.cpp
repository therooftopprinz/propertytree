#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <propertytree/Client.hpp>
#include <propertytree/Property.hpp>

namespace propertytree
{

Client::Client(const ClientConfig& pConfig)
    : logger("propertytree::client.bin")
{
    mRunner = std::thread([this](){
            mReactor.run();
        });

    mFd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == mFd)
    {
        throw std::runtime_error(strerror(errno));
    }

    sockaddr_in server;
    std::memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = ntohs(pConfig.port);

    auto res = inet_pton(AF_INET, pConfig.ip.c_str(), &server.sin_addr.s_addr);

    if (-1 == res)
    {
        throw std::runtime_error(strerror(errno));
    }

    char loc[24];
    inet_ntop(AF_INET, &pConfig.ip, loc, sizeof(loc));

    res = connect(mFd, (sockaddr*)&server, sizeof(server));

    if (-1 == res)
    {
        throw std::runtime_error(strerror(errno));
    }

    if (!mReactor.addHandler(mFd, [this](){
            handleRead();
        }))
    {
        throw std::runtime_error("Failed to add client socket to EpollReactor!");
    }

    PropertyTreeProtocol message = PropertyTreeMessage{};
    auto& propertyTreeMessage = std::get<PropertyTreeMessage>(message);
    propertyTreeMessage.message = SigninRequest{};

    auto trId = addTransaction(std::move(message));
    auto response = waitTransaction(trId);

    if (cum::GetIndexByType<PropertyTreeMessages, SigninAccept>() != response.index())
    {
        throw std::runtime_error("signin failure!");
    }

    std::unique_lock<std::mutex> lg(mTreeMutex);
    mTree.emplace(0, std::make_shared<Node>("", std::shared_ptr<Node>(), 0));
}

Client::~Client()
{
    mReactor.stop();
    mRunner.join();
    close(mFd);
}

Property Client::root()
{
    std::unique_lock<std::mutex> lgTree(mTreeMutex);
    return Property(*this, mTree.find(0)->second);
}

Property Client::create(Property& pParent, const std::string& pName)
{
    LOGLESS_TRACE(logger);
    auto& node = pParent.node();

    std::unique_lock<std::mutex> lgChildren(node->childrenMutex);
    auto foundIt = node->children.find(pName);
    if (node->children.end() != foundIt)
    {
        return Property(*this, foundIt->second);
    }
    lgChildren.unlock();

    PropertyTreeProtocol message = PropertyTreeMessage{};
    auto& propertyTreeMessage = std::get<PropertyTreeMessage>(message);
    propertyTreeMessage.message = CreateRequest{};
    auto& createRequest = std::get<CreateRequest>(propertyTreeMessage.message);
    createRequest.name = pName;
    createRequest.parentUuid = node->uuid;

    auto trId = addTransaction(std::move(message));
    auto response = waitTransaction(trId);

    if (cum::GetIndexByType<PropertyTreeMessages, CreateReject>() == response.index())
    {
        auto foundIt = node->children.find(pName);
        if (node->children.end() != foundIt)
        {
            return Property(*this, foundIt->second);
        }
        return Property(*this, nullptr);
    }
    else if (cum::GetIndexByType<PropertyTreeMessages, CreateAccept>() == response.index())
    {
        auto& createAccept = std::get<CreateAccept>(response);
        std::unique_lock<std::mutex> lgTree(mTreeMutex);

        // Note: Case when TreeUpdateNotification came first.
        auto foundIt = mTree.find(createAccept.uuid);
        if (mTree.end() != foundIt)
        {
            return Property(*this, foundIt->second);
        }
        auto newNode = std::make_shared<Node>(pName, node, createAccept.uuid);
        mTree.emplace(createAccept.uuid, newNode);
        std::unique_lock<std::mutex> lgNode(node->childrenMutex);
        node->children.emplace(pName, newNode);
        lgTree.unlock();
        return Property(*this, newNode);
    }
    else
    {
        throw std::runtime_error("protocol error!");
    }
}

Property Client::get(Property& pParent, const std::string& pName, bool pRecursive)
{
    LOGLESS_TRACE(logger);
    auto& node = pParent.node();

    std::unique_lock<std::mutex> lg(node->childrenMutex);
    auto foundIt = node->children.find(pName);
    if (!pRecursive && node->children.end()!= foundIt)
    {
        return Property(*this, foundIt->second);
    }
    lg.unlock();


    PropertyTreeProtocol message = PropertyTreeMessage{};
    auto& propertyTreeMessage = std::get<PropertyTreeMessage>(message);
    propertyTreeMessage.message = TreeInfoRequest{};
    auto& treeInfoRequest = std::get<TreeInfoRequest>(propertyTreeMessage.message);
    treeInfoRequest.name = pName;
    treeInfoRequest.parentUuid = pParent.uuid();
    treeInfoRequest.recursive = pRecursive;

    auto trId = addTransaction(std::move(message));
    auto response = waitTransaction(trId);

    if (cum::GetIndexByType<PropertyTreeMessages, TreeInfoResponse>() == response.index())
    {
        auto& treeInfoResponse = std::get<TreeInfoResponse>(response);
        addNodes(treeInfoResponse.nodeToAddList);

        if ("." == pName)
        {
            return pParent;
        }

        auto nodeUuid = treeInfoResponse.nodeToAddList[0].uuid;

        std::unique_lock<std::mutex> lg(mTreeMutex);
        auto foundIt = mTree.find(nodeUuid);

        return Property(*this, foundIt->second);
    }
    else if (cum::GetIndexByType<PropertyTreeMessages, TreeInfoErrorResponse>() == response.index())
    {
        return Property(*this, nullptr);
    }
    else
    {
        throw std::runtime_error("protocol error!");
    }
}

void Client::handle(uint16_t, TreeUpdateNotification&& pMsg)
{
    addNodes(pMsg.nodeToAddList);
    removeNodes(pMsg.nodeToDelete);
}

void Client::handle(uint16_t, UpdateNotification&& pMsg)
{
    LOGLESS_TRACE(logger);
    std::unique_lock<std::mutex> lgTree(mTreeMutex);
    auto nodeIt = mTree.find(pMsg.uuid);
    if (mTree.end() == nodeIt)
    {
        return;
    }
    auto node = nodeIt->second;
    lgTree.unlock();

    std::unique_lock<std::mutex> lgData(node->dataMutex);

    if (node->data.size() == pMsg.data.size())
    {
        std::memcpy(node->data.data(), pMsg.data.data(), pMsg.data.size());
    }
    else
    {
        node->data = std::move(pMsg.data);
    }

    lgData.unlock();
    std::unique_lock<std::mutex> lgUpdateHandler(node->updateHandlerMutex);
    if (node->updateHandler)
    {
        node->updateHandler();
    }
}

void Client::handle(uint16_t pTransactionId, RpcRequest&& pMsg)
{
    LOGLESS_TRACE(logger);
    PropertyTreeProtocol message = PropertyTreeMessage{};
    auto& propertyTreeMessage = std::get<PropertyTreeMessage>(message);
    propertyTreeMessage.message = RpcReject{};
    propertyTreeMessage.transactionId = pTransactionId;
    auto& rpcReject = std::get<RpcReject>(propertyTreeMessage.message);

    std::unique_lock<std::mutex> lg(mTreeMutex);
    auto nodeIt = mTree.find(pMsg.uuid);
    if (mTree.end() == nodeIt)
    {
        rpcReject.cause = Cause::NOT_FOUND;
        send(std::move(message));
        return;
    }
    auto node = nodeIt->second;
    lg.unlock();

    std::unique_lock<std::mutex> lgHandler(node->rcpHandlerMutex);
    if (node->rcpHandler)
    {
        propertyTreeMessage.message = RpcAccept{};
        auto& rcpAccept = std::get<RpcAccept>(propertyTreeMessage.message);

        auto rv = node->rcpHandler(bfc::BufferView((std::byte*)pMsg.param.data(), pMsg.param.size()));
        rcpAccept.value.resize(rv.size());
        std::memcpy(rcpAccept.value.data(), rv.data(), rv.size());
        send(std::move(message));
        return;
    }
    lgHandler.unlock();

    rpcReject.cause = Cause::NO_HANDLER;
    send(std::move(message));
    return;
}

void Client::commit(Property& pProp)
{
    LOGLESS_TRACE(logger);
    PropertyTreeProtocol message = PropertyTreeMessage{};
    auto& propertyTreeMessage = std::get<PropertyTreeMessage>(message);
    propertyTreeMessage.message = SetValueRequest{};
    auto& setValueRequest = std::get<SetValueRequest>(propertyTreeMessage.message);
    setValueRequest.uuid = pProp.uuid();

    for (auto i=0u; i<pProp.node()->data.size(); i++)
    {
        setValueRequest.data.emplace_back((uint8_t)pProp.node()->data.data()[i]);
    }

    auto trId = addTransaction(std::move(message));
    auto response = waitTransaction(trId);

    if (cum::GetIndexByType<PropertyTreeMessages, SetValueAccept>() != response.index())
    {
        throw std::runtime_error("protocol error!");
    }
}

void Client::fetch(Property& pProp)
{
    LOGLESS_TRACE(logger);
    PropertyTreeProtocol message = PropertyTreeMessage{};
    auto& propertyTreeMessage = std::get<PropertyTreeMessage>(message);
    propertyTreeMessage.message = GetRequest{};
    auto& getRequest = std::get<GetRequest>(propertyTreeMessage.message);
    getRequest.uuid = pProp.uuid();

    auto trId = addTransaction(std::move(message));
    auto response = waitTransaction(trId);

    if (cum::GetIndexByType<PropertyTreeMessages, GetAccept>() == response.index())
    {
        auto& getAccept = std::get<GetAccept>(response);
        auto& node = *pProp.node();
        std::unique_lock<std::mutex> lg(node.dataMutex);
        if (node.data.size() == getAccept.data.size())
        {
            std::memcpy(node.data.data(), getAccept.data.data(), getAccept.data.size());
        }
        else
        {
            node.data = std::move(getAccept.data);
        }
    }
    else if (cum::GetIndexByType<PropertyTreeMessages, GetReject>() != response.index())
    {
        throw std::runtime_error("protocol error!");
    }
}

bool Client::subscribe(Property& pProp)
{
    LOGLESS_TRACE(logger);
    PropertyTreeProtocol message = PropertyTreeMessage{};
    auto& propertyTreeMessage = std::get<PropertyTreeMessage>(message);
    propertyTreeMessage.message = SubscribeRequest{};
    auto& subscribeRequest = std::get<SubscribeRequest>(propertyTreeMessage.message);
    subscribeRequest.uuid = pProp.uuid();

    auto trId = addTransaction(std::move(message));
    auto response = waitTransaction(trId);

    if (cum::GetIndexByType<PropertyTreeMessages, SubscribeResponse>() == response.index())
    {
        auto& subscribeResponse = std::get<SubscribeResponse>(response);
        return subscribeResponse.cause == Cause::OK;
    }
    else
    {
        throw std::runtime_error("protocol error!");
    }
}

bool Client::unsubscribe(Property& pProp)
{
    LOGLESS_TRACE(logger);
    PropertyTreeProtocol message = PropertyTreeMessage{};
    auto& propertyTreeMessage = std::get<PropertyTreeMessage>(message);
    propertyTreeMessage.message = UnsubscribeRequest{};
    auto& unsubscribeRequest = std::get<UnsubscribeRequest>(propertyTreeMessage.message);
    unsubscribeRequest.uuid = pProp.uuid();

    auto trId = addTransaction(std::move(message));
    auto response = waitTransaction(trId);

    if (cum::GetIndexByType<PropertyTreeMessages, UnsubscribeResponse>() == response.index())
    {
        auto& unsubscribeResponse = std::get<UnsubscribeResponse>(response);
        return unsubscribeResponse.cause == Cause::OK;
    }
    else
    {
        throw std::runtime_error("protocol error!");
    }
}

bool Client::destroy(Property& pProp)
{
    LOGLESS_TRACE(logger);
    PropertyTreeProtocol message = PropertyTreeMessage{};
    auto& propertyTreeMessage = std::get<PropertyTreeMessage>(message);
    propertyTreeMessage.message = DeleteRequest{};
    auto& deleteRequest = std::get<DeleteRequest>(propertyTreeMessage.message);
    deleteRequest.uuid = pProp.uuid();

    auto trId = addTransaction(std::move(message));
    auto response = waitTransaction(trId);

    if (cum::GetIndexByType<PropertyTreeMessages, DeleteResponse>() == response.index())
    {
        auto& deleteResponse = std::get<DeleteResponse>(response);
        if (deleteResponse.cause != Cause::OK)
        {
            return false;
        }
        auto parent = pProp.node()->parent.lock();
        auto name = pProp.node()->name;
        if (parent)
        {
            parent->children.erase(name);
        }
        return true;
    }
    else
    {
        throw std::runtime_error("protocol error!");
    }
}

void Client::beat()
{
    LOGLESS_TRACE(logger);
    PropertyTreeProtocol message = PropertyTreeMessage{};
    auto& propertyTreeMessage = std::get<PropertyTreeMessage>(message);
    propertyTreeMessage.message = HearbeatRequest{};

    auto trId = addTransaction(std::move(message));
    auto response = waitTransaction(trId);

    if (cum::GetIndexByType<PropertyTreeMessages, HearbeatResponse>() != response.index())
    {
        throw std::runtime_error("protocol error!");
    }
}


std::vector<uint8_t> Client::call(Property& pProp, const bfc::BufferView& pValue)
{
    LOGLESS_TRACE(logger);
    PropertyTreeProtocol message = PropertyTreeMessage{};
    auto& propertyTreeMessage = std::get<PropertyTreeMessage>(message);
    propertyTreeMessage.message = RpcRequest{};
    auto& rpcRequest = std::get<RpcRequest>(propertyTreeMessage.message);
    rpcRequest.uuid = pProp.uuid();

    rpcRequest.param.reserve(pValue.size());

    for (auto i=0u; i<pValue.size(); i++)
    {
        rpcRequest.param.emplace_back((uint8_t)pValue.data()[i]);
    }

    auto trId = addTransaction(std::move(message));
    auto response = waitTransaction(trId);

    if (cum::GetIndexByType<PropertyTreeMessages, RpcAccept>() == response.index())
    {
        auto& rpcAccept = std::get<RpcAccept>(response);
        return std::move(rpcAccept.value);
    }
    else if (cum::GetIndexByType<PropertyTreeMessages, RpcAccept>() == response.index())
    {
        return {};
    }
    else
    {
        throw std::runtime_error("protocol error!");
    }
}

void Client::setTreeAddHandler(std::function<void(Property)> pHandler)
{
    std::unique_lock<std::mutex> lgTreeHandlerMutex(mmTreeHandlerMutex);
    mTreeAddHandler = std::move(pHandler);
}

void Client::setTreeRemoveHandler(std::function<void(Property)> pHandler)
{
    std::unique_lock<std::mutex> lgTreeHandlerMutex(mmTreeHandlerMutex);
    mTreeRemoveHandler = std::move(pHandler);
}

void Client::handleRead()
{
    LOGLESS_TRACE(logger);
    int readSize = 0;
    if (WAIT_HEADER == mReadState)
    {
        readSize = 2;
    }
    else
    {
        readSize = mExpectedReadSize - mBuffIdx;
    }

    auto res = read(mFd, mBuff+mBuffIdx, readSize);

    if (res>0)
    {
        Logless(logger, "DBG Client: handleRead: size=_ data=_", res, BufferLog(res, mBuff+mBuffIdx));
    }

    if (-1 == res)
    {
        throw std::runtime_error(strerror(errno));
    }
    if (0 == res)
    {
        throw std::runtime_error("Server disconnected!");
    }

    mBuffIdx += res;

    if (WAIT_HEADER == mReadState)
    {
        std::memcpy(&mExpectedReadSize, mBuff, 2);
        mBuffIdx = 0;
        mReadState = WAIT_REMAINING;
        return;
    }

    if (mExpectedReadSize == mBuffIdx)
    {
        decodeMessage();
        mReadState = WAIT_HEADER;
        mBuffIdx = 0;
    }
}

void Client::decodeMessage()
{
    LOGLESS_TRACE(logger);
    PropertyTreeProtocol message;
    cum::per_codec_ctx context(mBuff, mBuffIdx);
    decode_per(message, context);

    std::string stred;
    str("root", message, stred, true);
    Logless(logger, "DBG Client: decode: decoded=_ raw=_", stred.c_str(), BufferLog(mBuffIdx, mBuff));

    std::visit([this](auto&& pMsg){
            handle(std::move(pMsg));
        }, std::move(message));
}

void Client::send(PropertyTreeProtocol&& pMsg)
{
    LOGLESS_TRACE(logger);

    std::byte buffer[512];
    auto& msgSize = *(new (buffer) uint16_t(0));
    cum::per_codec_ctx context(buffer+sizeof(msgSize), sizeof(buffer)-sizeof(msgSize));
    encode_per(pMsg, context);

    msgSize = sizeof(buffer)-context.size()-2;


    std::string stred;
    str("root", pMsg, stred, true);
    Logless(logger, "DBG Client: send: encoded=_ raw=_", stred.c_str(), BufferLog(msgSize+2, buffer));

    auto res = ::send(mFd, buffer, msgSize+2, 0);
    if (-1 == res)
    {
        throw std::runtime_error(strerror(errno));
    }
}

void Client::handle(PropertyTreeMessage&& pMsg)
{
    LOGLESS_TRACE(logger);
    auto trId = pMsg.transactionId;
    std::unique_lock<std::mutex> lg(mTransactionsMutex);
    auto foundIt = mTransactions.find(trId);

    if (mTransactions.end() != foundIt)
    {
        auto& transaction = foundIt->second;
        std::unique_lock<std::mutex> lg(transaction.mutex);
        transaction.satisfied = true;
        transaction.message = std::move(pMsg.message);
        transaction.cv.notify_one();
        return;
    }

    lg.unlock();
    std::visit([this, trId](auto&& pMsg){
        handle(trId, std::move(pMsg));
    }, std::move(pMsg.message));
}

void Client::handle(PropertyTreeMessageArray&& pMsg)
{
    for (auto& i : pMsg)
    {
        handle(std::move(i));
    }
}

void Client::removeNodes(const std::vector<uint64_t>& pNodes)
{
    LOGLESS_TRACE(logger);
    for (auto i : pNodes)
    {
        std::unique_lock<std::mutex> lgTree(mTreeMutex);
        auto foundIt = mTree.find(i);
        if (mTree.end() == foundIt)
        {
            continue;
        }

        std::unique_lock<std::mutex> lgTreeHandlerMutex(mmTreeHandlerMutex);
        if (mTreeRemoveHandler)
        {
            mTreeRemoveHandler(Property(*this, foundIt->second));
        }
        lgTreeHandlerMutex.unlock();

        auto parent = foundIt->second->parent.lock();

        if (parent)
        {
            std::unique_lock<std::mutex> lgChildren(parent->childrenMutex);
            parent->children.erase(foundIt->second->name);
        }

        mTree.erase(foundIt);
    }
}

void Client::addNodes(NamedNodeList& pNodeList)
{
    LOGLESS_TRACE(logger);
    for (auto& i : pNodeList)
    {
        std::unique_lock<std::mutex> lgTree(mTreeMutex);
        auto parentIt = mTree.find(i.parentUuid);
        auto nodeIt = mTree.find(i.uuid);

        if (mTree.end() == parentIt)
        {
            auto dummyParent = std::make_shared<Node>("ghost", std::weak_ptr<Node>{}, i.parentUuid);
            auto res = mTree.emplace(i.parentUuid, dummyParent);
            parentIt = res.first;
        }

        auto& parentNode = parentIt->second;

        if (mTree.end() != nodeIt)
        {
            auto oldParent = nodeIt->second.get()->parent;
            nodeIt->second.get()->name = i.name;
            nodeIt->second.get()->parent = parentIt->second;

            std::unique_lock<std::mutex> parentLg(parentNode->childrenMutex);
            parentNode->children[i.name] = nodeIt->second;
            parentLg.unlock();
            lgTree.unlock();

            if (!oldParent.lock())
            {
                std::unique_lock<std::mutex> lgTreeHandlerMutex(mmTreeHandlerMutex);
                if (mTreeAddHandler)
                {
                    mTreeAddHandler(Property(*this, nodeIt->second));
                }
            }
        }
        else
        {
            auto newNode = std::make_shared<Node>(i.name, parentNode, i.uuid);
            mTree.emplace(i.uuid, newNode);

            std::unique_lock<std::mutex> parentLg(parentNode->childrenMutex);
            parentNode->children[i.name] = newNode;
            parentLg.unlock();
            lgTree.unlock();

            std::unique_lock<std::mutex> lgTreeHandlerMutex(mmTreeHandlerMutex);
            if (mTreeAddHandler)
            {
                mTreeAddHandler(Property(*this, newNode));
            }
        }
    }
}

uint16_t Client::addTransaction(PropertyTreeProtocol&& pMsg)
{
    LOGLESS_TRACE(logger);
    uint16_t trId = mTransactioIdCtr.fetch_add(1);
    if (0xFFFF == trId)
    {
        trId = mTransactioIdCtr.fetch_add(1);
    }

    auto& message = std::get<PropertyTreeMessage>(pMsg);
    message.transactionId = trId;

    std::unique_lock<std::mutex> lg(mTransactionsMutex);
    mTransactions.emplace(std::piecewise_construct, std::forward_as_tuple(trId), std::forward_as_tuple());

    send(std::move(pMsg));

    return trId;
}

PropertyTreeMessages Client::waitTransaction(uint16_t pTrId)
{
    LOGLESS_TRACE(logger);
    std::unique_lock<std::mutex> lg(mTransactionsMutex);
    auto& transaction = mTransactions.find(pTrId)->second;
    lg.unlock();

    PropertyTreeMessages rv;

    {
        std::unique_lock<std::mutex> lg(transaction.mutex);

        transaction.cv.wait_for(lg, std::chrono::milliseconds(500), [this, &transaction](){
                return transaction.satisfied;
            });

        if (!transaction.satisfied)
        {
            throw std::runtime_error("transaction failure");
        }

        rv = std::move(transaction.message);
    }

    lg.lock();
    mTransactions.erase(pTrId);

    return rv;
}

} // propertytree
