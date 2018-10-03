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
    // TODO: if sending is done in handler an it's expecting reply it would expire waiting because this is the receive thread.
    logger::Logger log("MetaUpdateNotificationMessageHandler");

    protocol::MetaUpdateNotification notif;
    notif.unpackFrom(message);
    log << logger::DEBUG << "MetaUpdateNotification: " << notif.toString();
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
