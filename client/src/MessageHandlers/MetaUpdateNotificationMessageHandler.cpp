#include "MetaUpdateNotificationMessageHandler.hpp"

namespace ptree
{
namespace client
{

MetaUpdateNotificationMessageHandler::
    MetaUpdateNotificationMessageHandler(PTreeClient& pc, IEndPoint& ep):
        MessageHandler(pc, ep)
{}

void MetaUpdateNotificationMessageHandler::handle(protocol::MessageHeaderPtr header, BufferPtr message)
{
    logger::Logger log("MetaUpdateNotificationMessageHandler");

    protocol::MetaUpdateNotification notif;
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
