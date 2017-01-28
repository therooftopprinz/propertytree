#ifndef CLIENT_MESSAGEHANDLERS_MESSAGEHANDLER_HPP_
#define CLIENT_MESSAGEHANDLERS_MESSAGEHANDLER_HPP_

#include <interface/protocol.hpp>
#include <common/src/IEndPoint.hpp>
#include <common/src/Logger.hpp>
#include <client/src/TransactionsCV.hpp>

namespace ptree
{
namespace client
{

using common::IEndPoint;
using common::IEndPointPtr;

struct MessageHandler
{
    virtual ~MessageHandler() = default;
    virtual void handle(protocol::MessageHeader& header, Buffer& message);
};


} // namespace client
} // namespace ptree

#endif  // CLIENT_MESSAGEHANDLERS_MESSAGEHANDLER_HPP_