#include "MetaUpdateNotificationMessageHandler.hpp"

namespace ptree
{
namespace client
{

MetaUpdateNotificationMessageHandler::
    MetaUpdateNotificationMessageHandler(PTreeClient& pc, IEndPoint& ep):
        MessageHandler(pc, ep)
{}

void MetaUpdateNotificationMessageHandler::handle(protocol::MessageHeaderPtr, BufferPtr message)
{
    logger::Logger log("MetaUpdateNotificationMessageHandler");

    protocol::MetaUpdateNotification notif;
    notif.unpackFrom(*message);

    // Handle creates
    for (auto& i : notif.creations.get())
    {
        ptreeClient.triggerMetaUpdateWatchersCreate(i.path, i.propertyType);
    }
    // Handle deletes
    for (auto& i : notif.deletions.get())
    {
        ptreeClient.triggerMetaUpdateWatchersDelete(i.uuid);
    }
}

} // namespace client
} // namespace ptree
