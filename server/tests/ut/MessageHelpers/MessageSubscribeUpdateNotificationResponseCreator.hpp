#ifndef SERVER_MESSAGE_HELPER_MESSAGE_SUBSCRIBEUPDATENOTIFICATIONRESPONSE_CREATOR_HPP_
#define SERVER_MESSAGE_HELPER_MESSAGE_SUBSCRIBEUPDATENOTIFICATIONRESPONSE_CREATOR_HPP_

#include "MessageCreator.hpp"

namespace ptree
{
namespace server
{

/** ---------------------------------------------- MessageSubscribeUpdateNotificationResponseCreator
*   HEADER  | REPSONSE
*            (uint8_t)
*/

class MessageSubscribeUpdateNotificationResponseCreator : MessageCreator
{
public:
    MessageSubscribeUpdateNotificationResponseCreator(uint32_t transactionId):
        MessageCreator(transactionId)
    {}

    Buffer create();
    void setResponse(protocol::SubscribePropertyUpdateResponse::Response response);
    
private:
    protocol::SubscribePropertyUpdateResponse::Response response;
};

} // namespace server
} // namespace ptree

#endif  // SERVER_MESSAGE_HELPER_MESSAGE_SUBSCRIBEUPDATENOTIFICATIONRESPONSE_CREATOR_HPP_    