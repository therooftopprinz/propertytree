#ifndef SERVER_MESSAGE_HELPER_MESSAGE_DELETERESPONSE_SENDER_HPP_
#define SERVER_MESSAGE_HELPER_MESSAGE_DELETERESPONSE_SENDER_HPP_

#include "MessageSender.hpp"

namespace ptree
{
namespace server
{

/** ------------------------------------------------------------------   MessageDeleteResponseSender
*
*    HEADER | CREATION_STATUS | OBJECT_PASS
*               (uint8_t)       (uint32)
*/

class MessageDeleteResponseSender : public MessageSender
{
public:
    MessageDeleteResponseSender(uint32_t transactionId, IEndPointPtr endpoint):
        MessageSender(transactionId, endpoint)
    {}

    void send();
    void setResponse(protocol::DeleteResponse::Response response);
    
private:
    protocol::DeleteResponse::Response response;
};


} // namespace server
} // namespace ptree

#endif  // SERVER_MESSAGE_HELPER_MESSAGE_DELETERESPONSE_SENDER_HPP_