#ifndef CLIENT_MESSAGEHANDLERS_METAUPDATENOTIFICATIONMESSAGEHANDLER_HPP_
#define CLIENT_MESSAGEHANDLERS_METAUPDATENOTIFICATIONMESSAGEHANDLER_HPP_

#include "MessageHandler.hpp"

namespace ptree
{
namespace client
{

struct MetaUpdateNotificationMessageHandler : public MessageHandler
{
    MetaUpdateNotificationMessageHandler(PTreeClient& ps, IEndPoint& ep);
    void handle(protocol::MessageHeaderPtr header, BufferPtr message);
};


} // namespace client
} // namespace ptree

#endif  // CLIENT_MESSAGEHANDLERS_METAUPDATENOTIFICATIONMESSAGEHANDLER_HPP_