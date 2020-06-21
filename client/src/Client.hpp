#ifndef __PROPERTYTREE_HPP__
#define __PROPERTYTREE_HPP__

#include <thread>
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <unordered_map>

#include <bfc/EpollReactor.hpp>
#include <bfc/Buffer.hpp>

#include <interface/protocol.hpp>

#include <propertytree/Node.hpp>

namespace propertytree
{

struct Property;

struct ClientConfig
{
    std::string ip;
    uint16_t port;
};

struct Transaction
{
    bool satisfied = false;
    PropertyTreeMessages message;
    std::condition_variable cv;
    std::mutex mutex;
};

class Client
{
public:
    Client(const ClientConfig& pConfig);
    ~Client();

    Property root();
    Property create(Property& pParent, const std::string& pName);
    Property get(Property& pParent, const std::string& pName, bool pRecursive);
    void commit(Property& pProp);
    void fetch(Property& pProp);
    bool subscribe(Property&);
    bool unsubscribe(Property&);
    bool destroy(Property&);
    void beat();
    std::vector<uint8_t> call(Property&, const bfc::BufferView& pValue);

    void setTreeAddHandler(std::function<void(Property)> pHandler);
    void setTreeRemoveHandler(std::function<void(Property)> pHandler);

private:
    void send(PropertyTreeProtocol&& pMsg);

    void handle(PropertyTreeMessage&& pMsg);
    void handle(PropertyTreeMessageArray&& pMsg);

    template <typename T>
    void handle(uint16_t pTrId, T&& pMsg){}
    void handle(uint16_t pTrId, TreeUpdateNotification&& pMsg);
    void handle(uint16_t pTrId, UpdateNotification&& pMsg);
    void handle(uint16_t pTrId, RpcRequest&& pMsg);

    void removeNodes(const std::vector<uint64_t>& pNodes);
    void addNodes(NamedNodeList& pNodeList);
    
    void handleRead();
    void decodeMessage();

    uint16_t addTransaction(PropertyTreeProtocol&& pMsg);
    PropertyTreeMessages waitTransaction(uint16_t pTrId);

    bfc::EpollReactor mReactor;
    int mFd;

    std::byte mBuff[1024*64];
    uint16_t mBuffIdx = 0;
    enum ReadState {WAIT_HEADER, WAIT_REMAINING};
    ReadState mReadState = WAIT_HEADER;
    int mExpectedReadSize = 0;

    std::thread mRunner;
    std::unordered_map<uint64_t, std::shared_ptr<Node>>  mTree;
    std::mutex mTreeMutex;

    std::unordered_map<uint16_t, Transaction> mTransactions;
    std::mutex mTransactionsMutex;

    std::atomic_uint16_t mTransactioIdCtr{};

    std::function<void(Property)> mTreeAddHandler;
    std::function<void(Property)> mTreeRemoveHandler;
    std::mutex mmTreeHandlerMutex;
};

} // propertytree

#endif // __PROPERTYTREE_HPP__