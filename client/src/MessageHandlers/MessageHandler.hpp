#ifndef CLIENT_MESSAGEHANDLERS_MESSAGEHANDLER_HPP_
#define CLIENT_MESSAGEHANDLERS_MESSAGEHANDLER_HPP_

#include <common/src/IEndPoint.hpp>
#include <common/src/Logger.hpp>
#include <client/src/PTreeClient.hpp>

namespace ptree
{
namespace client
{

using common::IEndPoint;
using common::IEndPointPtr;

struct MessageHandler
{
    MessageHandler(PTreeClient& pc, IEndPoint& ep):
        ptreeClient(pc),
        endpoint(ep)
    {}

    virtual void handle(protocol::MessageHeaderPtr header, BufferPtr message);
    static Buffer createHeader(protocol::MessageType type, uint32_t payloadSize, uint32_t transactionId);
    template<class T>
    void messageSender(uint32_t tid, protocol::MessageType mtype, T& msg)
    {
        Buffer header = createHeader(mtype, msg.size(), tid);
        endpoint.send(header.data(), header.size());

        Buffer responseMessageBuffer = msg.getPacked();
        endpoint.send(responseMessageBuffer.data(), responseMessageBuffer.size());
    }

    PTreeClient& ptreeClient;
    IEndPoint& endpoint;
};


} // namespace client
} // namespace ptree

#endif  // CLIENT_MESSAGEHANDLERS_MESSAGEHANDLER_HPP_