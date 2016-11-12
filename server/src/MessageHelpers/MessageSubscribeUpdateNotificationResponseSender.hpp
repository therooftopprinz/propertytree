#ifndef SERVER_MESSAGE_HELPER_MESSAGE_SUBSCRIBEUPDATENOTIFICATIONRESPONSE_SENDER_HPP_
#define SERVER_MESSAGE_HELPER_MESSAGE_SUBSCRIBEUPDATENOTIFICATIONRESPONSE_SENDER_HPP_

#include "MessageSender.hpp"

namespace ptree
{
namespace server
{

/** ---------------------------------------------- MessageSubscribeUpdateNotificationResponseSender
*   HEADER  | REPSONSE
*            (uint8_t)
*/

class MessageSubscribeUpdateNotificationResponseSender : public MessageSender
{
public:
    MessageSubscribeUpdateNotificationResponseSender(uint32_t transactionId, 
        IEndPointPtr endpoint):
        MessageSender(transactionId, endpoint)
    {}

    void send();
    void setResponse(protocol::SubscribePropertyUpdateResponse::Response response);
    
private:
    protocol::SubscribePropertyUpdateResponse::Response response;
};



} // namespace server
} // namespace ptree

#endif  // SERVER_MESSAGE_HELPER_MESSAGE_SUBSCRIBEUPDATENOTIFICATIONRESPONSE_SENDER_HPP_