#ifndef CLIENT_MESSAGEHANDLERS_PROPERTYUPDATENOTIFICATIONMESSAGEHANDLER_HPP_
#define CLIENT_MESSAGEHANDLERS_PROPERTYUPDATENOTIFICATIONMESSAGEHANDLER_HPP_

#include "MessageHandler.hpp"
#include <client/src/LocalPTree.hpp>

namespace ptree
{
namespace client
{

class PropertyUpdateNotificationMessageHandler : public MessageHandler
{
public:
    PropertyUpdateNotificationMessageHandler(TransactionsCV& transactionsCV, LocalPTree& ptree);
    ~PropertyUpdateNotificationMessageHandler() = default;
    void handle(protocol::MessageHeader& header, Buffer& message);
private:
    TransactionsCV& transactionsCV;
    LocalPTree& ptree;
    logger::Logger log;
};

} // namespace client
} // namespace ptree
#endif  // CLIENT_MESSAGEHANDLERS_PROPERTYUPDATENOTIFICATIONMESSAGEHANDLER_HPP_