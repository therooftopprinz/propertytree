#ifndef SERVER_CLIENTSERVER_HPP_
#define SERVER_CLIENTSERVER_HPP_

#include <mutex>

#include <server/src/PTreeTcpServer.hpp>
#include <server/src/PTree.hpp>
#include <common/src/Logger.hpp>
#include <server/src/Types.hpp>

#include "IClientServer.hpp"
#include "PTreeOutgoing.hpp"

#include "MessageHandlerFactory.hpp"

namespace ptree
{
namespace server
{


/**
Inteface: IClientServer
Responsibilities:
    notifyCreation, notifyDeletion - notify the ClientServer about the change in meta
    notifyValueUpdate              - notify the ClientServer about the change in property value
    notifyRpcRequest               - notify the owning ClientServer about the incoming rpc request
    notifyRpcResponse              - notify the ClientServer about the rpc response
    setUpdateInterval              - outgoing update rate
    clientSigned                   - set ClientServer to signedIn
**/
struct  IClientServer : public std::enable_shared_from_this<IClientServer>
{
    virtual void notifyCreation(uint32_t uuid, protocol::PropertyType type, std::string path) = 0;
    virtual void notifyDeletion(uint32_t uuid) = 0;
    virtual void notifyValueUpdate(core::ValuePtr) = 0;

    virtual void notifyRpcRequest(protocol::Uuid uuid, uint64_t clientServerId, uint32_t transactionId, Buffer&& parameter) = 0;
    virtual void notifyRpcResponse(uint32_t transactionId, Buffer&& returnValue) = 0;

    // virtual void send(uint32_t transactionId, protocol::MessageType messageType, protocol::Message message); 

    virtual void setUpdateInterval(uint32_t interval) = 0;
    virtual void clientSigned() = 0;

};

using IClientServerPtr = std::shared_ptr<IClientServer>;

class IClientServerMonitor
{
public:
    IClientServerMonitor() {}
    virtual ~IClientServerMonitor() {}
    virtual void addClientServer(IClientServerPtr clientServer) = 0;
    virtual void removeClientServer(IClientServerPtr clientServer) = 0;
    virtual void notifyCreation(uint32_t uuid, protocol::PropertyType type, std::string path) = 0;
    virtual void notifyDeletion(uint32_t uuid) = 0;
    virtual void notifyRpcResponse(uint64_t csId, uint32_t transactionId, Buffer&& returnValue) = 0;
};

/** TODO: UT for monitor **/
class ClientServerMonitor : public IClientServerMonitor
{
public:
    ClientServerMonitor();
    ~ClientServerMonitor();
    void addClientServer(IClientServerPtr clientServer);
    void removeClientServer(IClientServerPtr clientServer);
    void notifyCreation(uint32_t uuid, protocol::PropertyType type, std::string path);
    void notifyDeletion(uint32_t uuid);
    void notifyRpcResponse(uint64_t clientServerId, uint32_t transactionId, Buffer&& returnValue);

private:
    /** NOTE: This will be kept as shared_ptr because weak doesnt have == operator for searching through the list.
        This won't cause circular references if ClientServer is removed from the monitor.**/
    std::list<IClientServerPtr> clientServers;
    std::mutex clientServersMutex;
    logger::Logger log;
};


struct MessageHandlerFactory;

class ClientServer : public IClientServer
{
public:
    ClientServer(IEndPointPtr endpoint, core::PTreePtr ptree, IClientServerMonitorPtr monitor):
        endpoint(endpoint),
        outgoing(std::make_shared<PTreeOutgoing>(endpoint)),
        ptree(ptree),
        monitor(monitor),
        handleIncomingIsRunning(false),
        handleOutgoingIsRunning(false),
        isSetup(false),
        isSignin(false),
        processMessageRunning(0),
        updateInterval(0),
        log(logger::Logger("ClientServer"))
    {
    }

    ClientServer(const ClientServer&) = delete;
    ClientServer(ClientServer&) = delete;
    void operator = (const ClientServer&) = delete;
    void operator = (ClientServer&) = delete;

    ~ClientServer()
    {
    }

    void setup();
    void teardown();

    void handleIncoming();
    void handleOutgoing();

    void notifyCreation(uint32_t uuid, protocol::PropertyType type, std::string path);
    void notifyDeletion(uint32_t uuid);
    void notifyValueUpdate(core::ValuePtr);

    void notifyRpcRequest(protocol::Uuid uuid, uint64_t clientServerId, uint32_t transactionId, Buffer&& parameter);
    void notifyRpcResponse(uint32_t transactionId, Buffer&& returnValue);

    void setUpdateInterval(uint32_t interval);
    void clientSigned();

    struct ActionTypeAndPath
    {
        enum class UpdateType {CREATE_OBJECT, DELETE_OBJECT};
        UpdateType utype;
        protocol::PropertyType ptype;
        std::string path;
    };

    typedef std::map<uint32_t, ActionTypeAndPath> UuidActionTypeAndPathMap;
    typedef std::pair<uint32_t,Buffer> TransactionIdBufferPair;

private:
    void processMessage(protocol::MessageHeaderPtr header, BufferPtr message);

    UuidActionTypeAndPathMap metaUpdateNotification;
    std::mutex metaUpdateNotificationMutex;

    std::list<core::ValuePtr> valueUpdateNotification;
    std::mutex valueUpdateNotificationMutex;

    IEndPointPtr endpoint;
    std::mutex sendLock;

    PTreeOutgoingPtr outgoing;

    core::PTreePtr ptree;
    IClientServerMonitorPtr monitor;
    bool handleIncomingIsRunning;
    bool handleOutgoingIsRunning;
    bool killHandleIncoming;
    bool killHandleOutgoing;
    bool isSetup;
    bool isSignin;
    uint32_t processMessageRunning;
    uint32_t updateInterval;
    enum class EIncomingState
    {
        WAIT_FOR_HEADER_EMPTY,
        WAIT_FOR_HEADER,
        WAIT_FOR_MESSAGE_EMPTY,
        WAIT_FOR_MESSAGE,
        ERROR_HEADER_TIMEOUT,
        ERROR_MESSAGE_TIMEOUT
    };
    EIncomingState incomingState;
    logger::Logger log;
};

} // namespace server
} // namespace ptree

#endif  // SERVER_CLIENTSERVER_HPP_