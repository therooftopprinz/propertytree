#ifndef SERVER_SERVERLET_MESSAGEHANDLERS_MESSAGEHANDLER_HPP_
#define SERVER_SERVERLET_MESSAGEHANDLERS_MESSAGEHANDLER_HPP_

#include <interface/protocol.hpp>
#include <common/src/IEndPoint.hpp>
#include <server/src/Types.hpp>


namespace ptree
{
namespace server
{

struct MessageHandler
{
    virtual void handle(protocol::MessageHeaderPtr header, BufferPtr message) = 0;
};


} // namespace server
} // namespace ptree

#endif  // SERVER_SERVERLET_MESSAGEHANDLERS_MESSAGEHANDLER_HPP_