#ifndef SERVER_MESSAGE_HELPER_MESSAGE_UNSUBSCRIBEUPDATENOTIFICATIONRESPONSE_SENDER_HPP_
#define SERVER_MESSAGE_HELPER_MESSAGE_UNSUBSCRIBEUPDATENOTIFICATIONRESPONSE_SENDER_HPP_

#include "MessageSender.hpp"

namespace ptree
{
namespace server
{

/** --------------------------------------------- MessageUnsubscribeUpdateNotificationResponseSender
*   HEADER  | REPSONSE
*            (uint8_t)
*/

class MessageUnsubscribeUpdateNotificationResponseSender : public MessageSender
{
public:
    MessageUnsubscribeUpdateNotificationResponseSender(uint32_t transactionId, 
        IEndPointPtr endpoint):
        MessageSender(transactionId, endpoint)
    {}

    void send();
    void setResponse(protocol::UnsubscribePropertyUpdateResponse::Response response);
    
private:
    protocol::UnsubscribePropertyUpdateResponse::Response response;
};



} // namespace server
} // namespace ptree

#endif  // SERVER_MESSAGE_HELPER_MESSAGE_SUBSCRIBEUPDATENOTIFICATIONRESPONSE_SENDER_HPP_