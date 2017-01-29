#include "MetaUpdateNotificationMessageHandler.hpp"

namespace ptree
{
namespace client
{

MetaUpdateNotificationMessageHandler::
    MetaUpdateNotificationMessageHandler(TransactionsCV& transactionsCV, LocalPTree& ptree):
        transactionsCV(transactionsCV), ptree(ptree)
{}

void MetaUpdateNotificationMessageHandler::handle(protocol::MessageHeader&, Buffer& message)
{
    logger::Logger log("MetaUpdateNotificationMessageHandler");

    protocol::MetaUpdateNotification notif;
    notif.unpackFrom(message);

    // Handle creates
    for (auto& i : notif.creations.get())
    {
        ptree.triggerMetaUpdateWatchersCreate(i.uuid, i.path, i.propertyType);
    }
    // Handle deletes
    for (auto& i : notif.deletions.get())
    {
        ptree.triggerMetaUpdateWatchersDelete(i.uuid);
    }
}

} // namespace client
} // namespace ptree
