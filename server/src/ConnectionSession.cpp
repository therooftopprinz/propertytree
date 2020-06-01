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

ConnectionSession::~ConnectionSession()
{
    close(mFd);
}

void ConnectionSession::send(const bfc::ConstBufferView& pBuffer)
{
    Logless("DBG ConnectionSession[_]: send: _", mFd, BufferLog(pBuffer.size(), pBuffer.data()));
    auto res = ::send(mFd, pBuffer.data(), pBuffer.size(), 0);
    if (-1 == res)
    {
        Logless("ERR ConnectionSession[_]: write error=_", mFd, strerror(errno));
        // TODO: this causes recursive locking in ProtocolHandler::mSessionsMutex 
        // mServer.onDisconnect(mFd);
        return;
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

    if (0 >= res)
    {
        Logless("DBG ConnectionSession[_]: read error=_", mFd, strerror(errno));
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
        Logless("DBG ConnectionSession[_]: receive: _", mFd, BufferLog(mBuffIdx, mBuff));

        mProto.onMsg(bfc::ConstBufferView(mBuff, mBuffIdx), shared_from_this());

        mReadState = WAIT_HEADER;
        mBuffIdx = 0;
    }
}

} // propertytree
