#ifndef __CONNECTION_SESSION_HPP__
#define __CONNECTION_SESSION_HPP__

#include <memory>

#include <logless/Logger.hpp>

#include <bfc/EpollReactor.hpp>

#include <interface/protocol.hpp>

#include <IServer.hpp>
#include <ProtocolHandler.hpp>
#include <IConnectionSession.hpp>

namespace propertytree
{

class ConnectionSession : public IConnectionSession, public std::enable_shared_from_this<ConnectionSession>
{
public:
    ConnectionSession(int pFd, IServer& pServer, ProtocolHandler& pProto);
    ~ConnectionSession();
    void handleRead();
private:
    void send(const bfc::ConstBufferView&);

    std::byte mBuff[512];
    uint16_t mBuffIdx = 0;
    enum ReadState {WAIT_HEADER, WAIT_REMAINING};
    ReadState mReadState = WAIT_HEADER;
    int mExpectedReadSize = 0;

    int mFd;
    IServer& mServer;
    ProtocolHandler& mProto;
};

} // propertytree

#endif // __CONNECTION_SESSION_HPP__
