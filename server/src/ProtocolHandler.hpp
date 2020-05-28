#ifndef __PROTOCOLHANDLER_HPP__
#define __PROTOCOLHANDLER_HPP__

#include <bfc/ThreadPool.hpp>
#include <bfc/Timer.hpp>
#include <bfc/Singleton.hpp>

#include <logless/Logger.hpp>

#include <interface/protocol.hpp>

#include <IConnectionSession.hpp>
#include <Node.hpp>

namespace propertytree
{

struct Session
{
    Session() = delete;
    Session(std::shared_ptr<IConnectionSession> & pSession)
        : connectionSession(pSession)
    {}

    std::shared_ptr<IConnectionSession> connectionSession;
    std::mutex connectionSessionMutex;
};

class ProtocolHandler
{
public:
    ProtocolHandler();

    void onDisconnect(IConnectionSession* pConnection);
    void onMsg(bfc::ConstBuffer pBuffer, std::shared_ptr<IConnectionSession> pConnection);

private:

    void onMsg(PropertyTreeMessage&& pMsg, std::shared_ptr<IConnectionSession>& pConnection);
    void onMsg(PropertyTreeMessageArray&& pMsg, std::shared_ptr<IConnectionSession>& pConnection);

    template <typename T>
    void handle(uint16_t pTransactionId, T&& pMsg, std::shared_ptr<IConnectionSession>& pConnection) {}
    void handle(uint16_t pTransactionId, SigninRequest&& pMsg, std::shared_ptr<IConnectionSession>& pConnection);
    void handle(uint16_t pTransactionId, CreateRequest&& pMsg, std::shared_ptr<IConnectionSession>& pConnection);
    void handle(uint16_t pTransactionId, TreeInfoRequest&& pMsg, std::shared_ptr<IConnectionSession>& pConnection);
    void handle(uint16_t pTransactionId, SetValueRequest&& pMsg, std::shared_ptr<IConnectionSession>& pConnection);
    void handle(uint16_t pTransactionId, GetRequest&& pMsg, std::shared_ptr<IConnectionSession>& pConnection);
    void handle(uint16_t pTransactionId, SubscribeRequest&& pMsg, std::shared_ptr<IConnectionSession>& pConnection);
    void handle(uint16_t pTransactionId, UnsubscribeRequest&& pMsg, std::shared_ptr<IConnectionSession>& pConnection);
    void handle(uint16_t pTransactionId, DeleteRequest&& pMsg, std::shared_ptr<IConnectionSession>& pConnection);


    template <typename T>
    void fillToAddListFromTree(T& pIe, std::shared_ptr<Node>& pNode, bool pRecursive);

    void send(const PropertyTreeProtocol& pMsg, std::shared_ptr<IConnectionSession>& pConnection);

    std::unordered_map<uint32_t, Session> mSessions;
    std::unordered_map<IConnectionSession*, uint32_t> mConnectionToSessionId;
    std::mutex mSessionsMutex;
    std::atomic_uint32_t mSessionIdCtr{};

    std::unordered_map<uint64_t, std::shared_ptr<Node>> mTree;
    std::mutex mTreeMutex;
    std::atomic_uint32_t mUuidCtr{};

    bfc::ThreadPool<>& mTp;
    bfc::Timer<>& mTimer;

};

} // propertytree

#endif // __PROTOCOLHANDLER_HPP__