#include "PropertyUpdateNotificationMessageHandler.hpp"

namespace ptree
{
namespace client
{

PropertyUpdateNotificationMessageHandler::
    PropertyUpdateNotificationMessageHandler(PTreeClient& pc, IEndPoint& ep):
        MessageHandler(pc, ep)
{}

void PropertyUpdateNotificationMessageHandler::handle(protocol::MessageHeaderPtr header, BufferPtr message)
{
    protocol::PropertyUpdateNotification notif;
    notif.unpackFrom(*message);

    for (auto& i : notif.propertyUpdateNotifications.get())
    {
        ptreeClient.handleUpdaNotification(i.uuid, std::move(i.data));
    }
}

} // namespace client
} // namespace ptree
