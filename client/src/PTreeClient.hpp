#ifndef CLIENT_PTREECLIENT_HPP_
#define CLIENT_PTREECLIENT_HPP_

#include <cassert>
#include <map>
#include <vector>
#include <string>
#include <mutex>
#include <thread>
#include <memory>
#include <atomic>
#include <algorithm>
#include <condition_variable>
#include <interface/protocol.hpp>
#include <common/src/Logger.hpp>
#include <common/src/IEndPoint.hpp>
#include <client/src/TransactionsCV.hpp>
#include <client/src/ClientOutgoing.hpp>
#include <client/src/ClientIncoming.hpp>

namespace ptree
{
namespace client
{

class ValueContainer;
typedef std::shared_ptr<ValueContainer> ValueContainerPtr;
class RpcContainer;
typedef std::shared_ptr<RpcContainer> RpcContainerPtr;


struct IMetaUpdateHandler;
class PTreeClient : public std::enable_shared_from_this<PTreeClient>
{
public:
    PTreeClient(common::IEndPointPtr endpoint);
    ~PTreeClient();

    // void signIn();
    // ValueContainerPtr createValue(std::string path, Buffer value);
    // RpcContainerPtr createRpc(std::string path, std::function<Buffer(Buffer&)> handler, std::function<void(Buffer&)> voidHandler);
    // bool createNode(std::string path);
    // ValueContainerPtr getValue(std::string& path);
    // RpcContainerPtr getRpc(std::string& path);
    // void addMetaWatcher(std::shared_ptr<IMetaUpdateHandler> handler);
    // void deleteMetaWatcher(std::shared_ptr<IMetaUpdateHandler> handler);

    // inline uint32_t getTransactionId()
    // {
    //     return transactionIdGenerator.get();
    // }

    // bool enableAutoUpdate(ValueContainerPtr&);
    // bool disableAutoUpdate(ValueContainerPtr&);

    // void setValue(ValueContainerPtr&, Buffer&);
    // template<typename T>
    // void setValue(ValueContainerPtr& vc, T& val)
    // {
    //     Buffer v(sizeof(T));
    //     *(T*)(v.data()) = val;
    //     vc->updateValue(std::move(v), true);
    //     sendSetValue(vc);
    // }

    // Buffer rpcRequest(RpcContainerPtr& rpc, Buffer& argument);

private:
    // void signIn(bool enableMetaUpdate, uint32_t updateRate);
    // bool deleteProperty(std::string path);

    // void handleUpdaNotification(protocol::Uuid uuid, Buffer&& value);

    // void handleRpcResponse(BufferPtr returnType, uint64_t calee, uint32_t transactionId);
    // // void installUpdateHandler(uint64_t id, std::function<void()> handler);
    // void notifyTransactionCV(uint32_t transactionId, BufferPtr);

    // void triggerMetaUpdateWatchersCreate(std::string& path, protocol::PropertyType propertyType);
    // void triggerMetaUpdateWatchersDelete(protocol::Uuid path);

    // void processMessage(protocol::MessageHeaderPtr header, BufferPtr message);
    // void handleIncoming();

    // void sendSignIn(int refreshRate, const std::list<protocol::SigninRequest::FeatureFlag> features);
    // ValueContainerPtr sendGetValue(protocol::Uuid uuid, ValueContainerPtr& vc);
    // std::tuple<protocol::Uuid, protocol::PropertyType> fetchMeta(std::string& path);
    // protocol::Uuid fetchMetaAndAddToLocal(std::string& path);
    // void sendSetValue(ValueContainerPtr& vc);
    // /*** TODO: Commonize these with message handler***/
    // Buffer createHeader(protocol::MessageType type, uint32_t payloadSize, uint32_t transactionId);
    // template<class T>
    // void messageSender(uint32_t tid, protocol::MessageType mtype, T& msg)
    // {
    //     std::lock_guard<std::mutex> guard(sendLock);
    //     Buffer header = createHeader(mtype, msg.size(), tid);
    //     endpoint->send(header.data(), header.size());
    //     Buffer responseMessageBuffer = msg.getPacked();
    //     endpoint->send(responseMessageBuffer.data(), responseMessageBuffer.size());
    // }

    // Buffer callRpc(protocol::Uuid uuid, Buffer& parameter);

    // template<typename T, typename M = std::mutex>
    // struct MutexedObject
    // {
    //     T object;
    //     M mutex;
    // };

    // ValueContainerPtr getLocalValue(protocol::Uuid uuid);
    // void insertLocalValue(protocol::Uuid uuid, ValueContainerPtr& value);
    // MutexedObject<std::map<protocol::Uuid, ValueContainerPtr>> values;


    // RpcContainerPtr getLocalRpc(protocol::Uuid uuid);
    // void insertLocalRpc(protocol::Uuid uuid, RpcContainerPtr& rpc);
    // MutexedObject<std::map<protocol::Uuid, RpcContainerPtr>> rpcs;

    // void addMeta(protocol::Uuid, std::string path, protocol::PropertyType type);
    // void removeMeta(protocol::Uuid);
    // std::string getPath(protocol::Uuid uuid);
    // protocol::Uuid getUuid(std::string path);
    // struct PTreeMeta
    // {
    //     PTreeMeta(){}
    //     PTreeMeta(std::string& path, protocol::PropertyType type):
    //         path(path), type(type)
    //     {}
    //     std::string path;
    //     protocol::PropertyType type;
    // };
    // PTreeMeta getMeta(protocol::Uuid uuid);

    // /** TODO: common memory for string key and meta path **/
    // struct MetaObjects
    // {
    //     std::map<protocol::Uuid, PTreeMeta> uuidMetaMap;
    //     std::map<std::string, protocol::Uuid> pathUuidMap;
    // };
    // MutexedObject<MetaObjects> metaMap;
    // struct TransactionCV
    // {
    //     TransactionCV():
    //         condition(false)
    //     {}
    //     std::mutex mutex;
    //     std::condition_variable cv;
    //     std::atomic<bool> condition;
    //     Buffer value;
    // };

    // std::shared_ptr<TransactionCV> addTransactionCV(uint32_t transactionId);
    // bool waitTransactionCV(uint32_t transactionId);

    // typedef std::map<uint32_t, std::shared_ptr<TransactionCV>> TrCVMap;
    // MutexedObject<TrCVMap> transactionIdCV;
    // TransactionIdGenerator transactionIdGenerator;
    // MutexedObject<std::list<std::shared_ptr<IMetaUpdateHandler>>> metaUpdateHandlers;

    // bool handleIncomingIsRunning;
    // bool handleOutgoingIsRunning;
    // bool killHandleIncoming;
    // uint32_t processMessageRunning;

    server::IEndPointPtr endpoint;
    TransactionsCV transactionsCV;
    ClientOutgoing outgoing;
    ClientIncoming incoming;
    logger::Logger log;
};

struct IMetaUpdateHandler
{
    IMetaUpdateHandler() = default;
    virtual ~IMetaUpdateHandler() = default;
    virtual void handleCreation(std::string path, protocol::PropertyType propertyType) = 0;
    virtual void handleDeletion(protocol::Uuid) = 0;
};

}
}

#endif  // CLIENT_PTREECLIENT_HPP_
