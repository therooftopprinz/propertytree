#ifndef SERVER_MESSAGE_HELPER_MESSAGE_UNSUBSCRIBEUPDATENOTIFICATIONRESPONSE_CREATOR_HPP_
#define SERVER_MESSAGE_HELPER_MESSAGE_UNSUBSCRIBEUPDATENOTIFICATIONRESPONSE_CREATOR_HPP_

#include "MessageCreator.hpp"

namespace ptree
{
namespace server
{

/** ---------------------------------------------- MessageUnsubscribeUpdateNotificationResponseCreator
*   HEADER  | REPSONSE
*            (uint8_t)
*/

class MessageUnsubscribeUpdateNotificationResponseCreator : MessageCreator
{
public:
    MessageUnsubscribeUpdateNotificationResponseCreator(uint32_t transactionId):
        MessageCreator(transactionId)
    {}

    Buffer create();
    void setResponse(protocol::UnsubscribePropertyUpdateResponse::Response response);
    
private:
    protocol::UnsubscribePropertyUpdateResponse::Response response;
};

} // namespace server
} // namespace ptree

#endif  // SERVER_MESSAGE_HELPER_MESSAGE_UNSUBSCRIBEUPDATENOTIFICATIONRESPONSE_CREATOR_HPP_    