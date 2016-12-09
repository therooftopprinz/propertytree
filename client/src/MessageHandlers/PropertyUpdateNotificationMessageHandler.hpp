#ifndef CLIENT_MESSAGEHANDLERS_PROPERTYUPDATENOTIFICATIONMESSAGEHANDLER_HPP_
#define CLIENT_MESSAGEHANDLERS_PROPERTYUPDATENOTIFICATIONMESSAGEHANDLER_HPP_

#include "MessageHandler.hpp"

namespace ptree
{
namespace client
{

struct PropertyUpdateNotificationMessageHandler : public MessageHandler
{
    PropertyUpdateNotificationMessageHandler(PTreeClient& ps, IEndPoint& ep);
    void handle(protocol::MessageHeaderPtr header, BufferPtr message);
};


} // namespace client
} // namespace ptree

#endif  // CLIENT_MESSAGEHANDLERS_PROPERTYUPDATENOTIFICATIONMESSAGEHANDLER_HPP_