#ifndef SERVER_MESSAGE_HELPER_MESSAGE_PROPERTYUPDATENOTIFICATION_SENDER_HPP_
#define SERVER_MESSAGE_HELPER_MESSAGE_PROPERTYUPDATENOTIFICATION_SENDER_HPP_

#include "MessageSender.hpp"

namespace ptree
{
namespace server
{

/** ------------------------------------------------------- MessagePropertyUpdateNotificationSender
*   HEADER  | (   UUID    |    SIZE    |  DATA   )
*               (uint8_t)   (uint32_t)     []
*/


class MessagePropertyUpdateNotificationSender : public MessageSender
{
public:
    MessagePropertyUpdateNotificationSender(std::list<core::ValuePtr>& valueUpdate,
        IEndPointPtr endpoint):
        MessageSender((uint32_t)-1, endpoint),
        valueUpdate(valueUpdate)
    {}

    void send();
    uint32_t getMessageSize();

private:
    const std::list<core::ValuePtr>& valueUpdate;
};

} // namespace server
} // namespace ptree

#endif  // SERVER_MESSAGE_HELPER_MESSAGE_PROPERTYUPDATENOTIFICATION_CREATOR_HPP_