#ifndef SERVER_SERVERLET_MESSAGEHANDLERS_MESSAGEHANDLER_HPP_
#define SERVER_SERVERLET_MESSAGEHANDLERS_MESSAGEHANDLER_HPP_

#include <common/src/IEndPoint.hpp>
#include <server/src/PTree.hpp>
#include <server/src/Types.hpp>
#include <common/src/Logger.hpp>

namespace ptree
{
namespace server
{

// g++ buggy case?    
// struct ClientServer;
// struct IEndPoint;
// struct IClientServerMonitor;
// namespace core
// {
//     struct PTree;
// }

struct ClientServer;
struct MessageHandler
{
    MessageHandler(ClientServer& cs, IEndPoint& ep, core::PTree& pt, IClientServerMonitor&  csmon):
        clientServer(cs),
        endpoint(ep),
        ptree(pt),
        monitor(csmon)
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

    ClientServer& clientServer;
    IEndPoint& endpoint;
    core::PTree& ptree;
    IClientServerMonitor& monitor;
};


} // namespace server
} // namespace ptree

#endif  // SERVER_SERVERLET_MESSAGEHANDLERS_MESSAGEHANDLER_HPP_