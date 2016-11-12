#ifndef SERVER_MESSAGE_HELPER_MESSAGE_UNSUBSCRIBEUPDATENOTIFICATIONREQUEST_CREATOR_HPP_
#define SERVER_MESSAGE_HELPER_MESSAGE_UNSUBSCRIBEUPDATENOTIFICATIONREQUEST_CREATOR_HPP_

#include "MessageCreator.hpp"

namespace ptree
{
namespace server
{

/** ----------------------------------------------- MessageUnsubscribeUpdateNotificationRequestCreator
*   HEADER  |    UUID  
*             (uint32_t)
*/

class MessageUnsubscribeUpdateNotificationRequestCreator : public MessageCreator
{
public:
    MessageUnsubscribeUpdateNotificationRequestCreator(uint32_t transactionId):
        MessageCreator(transactionId)
    {}

    Buffer create();
    void setUuid(uint32_t uuid);
    
private:
    uint32_t uuid = 0;
};

} // namespace server
} // namespace ptree

#endif  // SERVER_MESSAGE_HELPER_MESSAGE_UNSUBSCRIBEUPDATENOTIFICATIONREQUEST_CREATOR_HPP_