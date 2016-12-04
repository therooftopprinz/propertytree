#ifndef SERVER_SERVERLET_MESSAGEHANDLERS_MESSAGEHANDLER_HPP_
#define SERVER_SERVERLET_MESSAGEHANDLERS_MESSAGEHANDLER_HPP_

#include <common/src/IEndPoint.hpp>
#include <common/src/Logger.hpp>

namespace ptree
{
namespace client
{


struct PTreeClient;
struct MessageHandler
{
    MessageHandler(PTreeClient& pc, IEndPoint& ep):
        clientServer(cs),
        endpoint(ep)
    {}

    virtual void handle(protocol::MessageHeaderPtr header, BufferPtr message);
    static Buffer createHeader(protocol::MessageType type, uint32_t payloadSize, uint32_t transactionId);
    template<class T>
    void messageSender(uint32_t tid, protocol::MessageType mtype, T& msg)
    {
        Buffer header = createHeader(mtype, msg.size(), tid);
        endpoint.send(header.data(), header.size());

        Buffer responseMessageBuffer(msg.size());
        protocol::BufferView responseMessageBufferView(responseMessageBuffer);
        protocol::Encoder en(responseMessageBufferView);
        msg >> en;
        endpoint.send(responseMessageBuffer.data(), responseMessageBuffer.size());
    }

    PTreeClient& ptreeClient;
    IEndPoint& endpoint;
};


} // namespace server
} // namespace ptree

#endif  // SERVER_SERVERLET_MESSAGEHANDLERS_MESSAGEHANDLER_HPP_