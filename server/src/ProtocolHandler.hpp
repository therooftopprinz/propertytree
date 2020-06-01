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
};

class ProtocolHandler
{
public:
    ProtocolHandler(bfc::LightFn<void()> pTerminator);

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

    template<typename T>
    void handleRpc(uint16_t pTransactionId, T&& pMsg, std::shared_ptr<IConnectionSession>& pConnection);
    void handle(uint16_t pTransactionId, RpcRequest&& pMsg, std::shared_ptr<IConnectionSession>& pConnection);
    void handle(uint16_t pTransactionId, RpcAccept&& pMsg, std::shared_ptr<IConnectionSession>& pConnection);
    void handle(uint16_t pTransactionId, RpcReject&& pMsg, std::shared_ptr<IConnectionSession>& pConnection);

    void handle(uint16_t pTransactionId, HearbeatRequest&& pMsg, std::shared_ptr<IConnectionSession>& pConnection);

    template <typename T>
    void fillToAddListFromTree(T& pIe, std::shared_ptr<Node>& pNode, bool pRecursive);

    size_t encode(const PropertyTreeProtocol& pMsg, std::byte* pData, size_t pSize);
    void send(const PropertyTreeProtocol& pMsg, std::shared_ptr<IConnectionSession>& pConnection);
    void send(const std::byte* pData, size_t pSize, std::shared_ptr<IConnectionSession>& pConnection);


    // mSessions: <SessionId, Session>
    std::unordered_map<uint32_t, std::shared_ptr<Session>> mSessions;
    std::unordered_map<IConnectionSession*, uint32_t> mConnectionToSessionId;
    uint32_t mSessionIdCtr{};

    uint32_t mTrIdCtr{};
    // mTrIdTranslation: TrId Req - Resp Translation Table: map<DestinationTrId, <SourceSessionId, SourceTrId>>
    std::unordered_map<uint16_t, std::pair<uint32_t, uint16_t>> mTrIdTranslation;

    std::unordered_map<uint64_t, std::shared_ptr<Node>> mTree;
    uint32_t mUuidCtr{};

    bfc::LightFn<void()> mTerminator;

};

} // propertytree

#endif // __PROTOCOLHANDLER_HPP__
