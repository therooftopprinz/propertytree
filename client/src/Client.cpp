#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <propertytree/Client.hpp>
#include <propertytree/Property.hpp>

namespace propertytree
{

Client::Client(const ClientConfig& pConfig)
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
    mTree.emplace(0, std::make_shared<Node>(std::shared_ptr<Node>(), 0));
}

Client::~Client()
{
    close(mFd);

    mReactor.stop();
    mRunner.join();
}

Property Client::root()
{
    return Property(*this, mTree.find(0)->second);
}

Property Client::create(Property& pParent, std::string pName)
{
    PropertyTreeProtocol message = PropertyTreeMessage{};
    auto& propertyTreeMessage = std::get<PropertyTreeMessage>(message);
    propertyTreeMessage.message = CreateRequest{};
    auto& createRequest = std::get<CreateRequest>(propertyTreeMessage.message);
    createRequest.name = pName;
    createRequest.parentUuid = pParent.node()->uuid;

    auto trId = addTransaction(std::move(message));
    auto response = waitTransaction(trId);

    auto node = pParent.node();

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
        std::unique_lock<std::mutex> lg(mTreeMutex);
        auto newNode = std::make_shared<Node>(pParent.node(), createAccept.uuid);
        mTree.emplace(createAccept.uuid, newNode);
        lg.unlock();
        std::unique_lock<std::mutex> lgNode(node->mutex);
        node->children.emplace(pName, newNode);
        return Property(*this, newNode);
    }
    else
    {
        throw std::runtime_error("protocol error!");
    }
}

Property Client::get(Property& pParent, std::string pName)
{
    auto parentNode = pParent.node();
    std::unique_lock<std::mutex> lg(parentNode->mutex);
    auto foundIt = parentNode->children.find(pName);
    if (parentNode->children.end()!= foundIt)
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
    treeInfoRequest.recursive = true;

    auto trId = addTransaction(std::move(message));
    auto response = waitTransaction(trId);

    if (cum::GetIndexByType<PropertyTreeMessages, TreeInfoResponse>() == response.index())
    {
        auto& treeInfoResponse = std::get<TreeInfoResponse>(response);
        addNodes(treeInfoResponse.nodeToAddList);
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

void Client::handle(uint16_t pTrId, TreeUpdateNotification&& pMsg)
{
    addNodes(pMsg.nodeToAddList);
}

void Client::commit(Property& pProp)
{
    PropertyTreeProtocol message = PropertyTreeMessage{};
    auto& propertyTreeMessage = std::get<PropertyTreeMessage>(message);
    propertyTreeMessage.message = SetValueIndication{};
    auto& setValueIndication = std::get<SetValueIndication>(propertyTreeMessage.message);
    setValueIndication.uuid = pProp.uuid();

    for (auto i=0u; i<pProp.node()->data.size(); i++)
    {
        setValueIndication.data.emplace_back((uint8_t)pProp.node()->data.data()[i]);
    }

    send(std::move(message));
}

void Client::fetch(Property& pProp)
{
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
        std::unique_lock<std::mutex> lg(node.mutex);
        if (node.data.size() == getAccept.data.size())
        {
            std::memcpy(node.data.data(), getAccept.data.data(), getAccept.data.size());
        }
        else
        {
            auto buff = new std::byte[getAccept.data.size()];
            std::memcpy(buff, getAccept.data.data(), getAccept.data.size());
            node.data = bfc::Buffer(buff, getAccept.data.size());
        }
    }
    else if (cum::GetIndexByType<PropertyTreeMessages, GetReject>() != response.index())
    {
        throw std::runtime_error("protocol error!");
    }
}

bool Client::subscribe(Property&)
{
    return false;
}

bool Client::unsubscribe(Property&)
{
    return false;
}

void Client::set(Property&, bfc::BufferView pValue)
{

}

Buffer Client::call(Property&, bfc::BufferView pValue)
{
    return {};
}

void Client::handleRead()
{
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
    PropertyTreeProtocol message;
    cum::per_codec_ctx context(mBuff, mBuffIdx);
    decode_per(message, context);

    std::visit([this](auto&& pMsg){
            handle(std::move(pMsg));
        }, std::move(message));
}

void Client::send(PropertyTreeProtocol&& pMsg)
{
        std::byte buffer[512];
        auto& msgSize = *(new (buffer) uint16_t(0));
        cum::per_codec_ctx context(buffer+sizeof(msgSize), sizeof(buffer)-sizeof(msgSize));
        encode_per(pMsg, context);

        msgSize = sizeof(buffer)-context.size()-2;

        auto res = ::send(mFd, buffer, msgSize+2, 0);
        if (-1 == res)
        {
            throw std::runtime_error(strerror(errno));
        }
}

void Client::handle(PropertyTreeMessage&& pMsg)
{
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

void Client::addNodes(NamedNodeList& pNodeList)
{
    for (auto& i : pNodeList)
    {
        std::unique_lock<std::mutex> lg(mTreeMutex);
        auto foundIt = mTree.find(i.parentUuid);
        if (mTree.end() == foundIt)
        {
            continue;
        }
        auto& parentNode = foundIt->second;
        lg.unlock();
        std::unique_lock<std::mutex> parentLg(parentNode->mutex);
        auto newNode = std::make_shared<Node>(parentNode, i.uuid);
        parentNode->children.emplace(i.name, newNode);
        std::unique_lock<std::mutex> lgTree(mTreeMutex);
        mTree.emplace(i.uuid, newNode);
    }
}

uint16_t Client::addTransaction(PropertyTreeProtocol&& pMsg)
{
    uint16_t trId = mTransactioIdCtr.fetch_add(1);

    auto& message = std::get<PropertyTreeMessage>(pMsg);
    message.transactionId = trId;

    std::unique_lock<std::mutex> lg(mTransactionsMutex);
    mTransactions.emplace(std::piecewise_construct, std::forward_as_tuple(trId), std::forward_as_tuple());

    send(std::move(pMsg));

    return trId;
}

PropertyTreeMessages Client::waitTransaction(uint16_t pTrId)
{
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
