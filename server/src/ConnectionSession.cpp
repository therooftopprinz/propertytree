#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <ConnectionSession.hpp>

namespace propertytree
{


ConnectionSession::ConnectionSession(int pFd, IServer& pServer, ProtocolHandler& pProto)
    : mFd(pFd)
    , mServer(pServer)
    , mProto(pProto)

{
}

void ConnectionSession::send(const bfc::ConstBufferView& pBuffer)
{
    Logless("ConnectionSession[_]: send: _", mFd, BufferLog(pBuffer.size(), pBuffer.data()));
    auto res = ::send(mFd, pBuffer.data(), pBuffer.size(), 0);
    if (-1 == res)
    {
        throw std::runtime_error(strerror(errno));
    }
}

void ConnectionSession::handleRead()
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
        mServer.onDisconnect(mFd);
        return;
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
        auto raw = new std::byte[mBuffIdx];
        bfc::ConstBuffer buff(raw, mBuffIdx);
        std::memcpy(raw, mBuff, mBuffIdx);

        Logless("ConnectionSession[_]: receive: _", mFd, BufferLog(buff.size(), buff.data()));

        mProto.onMsg(std::move(buff), shared_from_this());

        mReadState = WAIT_HEADER;
        mBuffIdx = 0;
    }
}

} // propertytree
