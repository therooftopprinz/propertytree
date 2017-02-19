#include "PropertyUpdateNotificationMessageHandler.hpp"
#include <client/src/LocalPTree.hpp>

namespace ptree
{
namespace client
{

PropertyUpdateNotificationMessageHandler::
    PropertyUpdateNotificationMessageHandler(TransactionsCV& transactionsCV, LocalPTree& ptree):
        transactionsCV(transactionsCV), ptree(ptree), log("PropertyUpdateNotificationMessageHandler")
{}

void PropertyUpdateNotificationMessageHandler::handle(protocol::MessageHeader&, Buffer& message)
{
    protocol::PropertyUpdateNotification notif;
    notif.unpackFrom(message);
    log << logger::DEBUG << "PropertyUpdateNotification: " << notif.toString();

    for (auto& i : notif.propertyUpdateNotifications.get())
    {
        ptree.handleUpdaNotification(i.uuid, std::move(i.data));
    }
}

} // namespace client
} // namespace ptree
