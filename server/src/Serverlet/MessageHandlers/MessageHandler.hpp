#ifndef SERVER_SERVERLET_MESSAGEHANDLERS_MESSAGEHANDLER_HPP_
#define SERVER_SERVERLET_MESSAGEHANDLERS_MESSAGEHANDLER_HPP_

#include <server/src/PTree.hpp>
#include <server/src/Types.hpp>
#include <server/src/Logger.hpp>

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

    ClientServer& clientServer;
    IEndPoint& endpoint;
    core::PTree& ptree;
    IClientServerMonitor& monitor;
};


} // namespace server
} // namespace ptree

#endif  // SERVER_SERVERLET_MESSAGEHANDLERS_MESSAGEHANDLER_HPP_