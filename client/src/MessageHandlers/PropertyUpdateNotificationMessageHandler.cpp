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
    logger::Logger log("PropertyUpdateNotificationMessageHandler");

    protocol::PropertyUpdateNotification notif;
    notif.unpackFrom(*message);

    // Handle creates
    // for (auto& i : *notif.creations)
    // {
    //     // if on path on watch list - call watcher
    // }
    // // Handle deletes
    // for (auto& i : *notif.deletions)
    // {

    // }
}

} // namespace client
} // namespace ptree
