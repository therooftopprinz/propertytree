#ifndef SERVER_MESSAGE_HELPER_MESSAGE_CREATERESPONSE_SENDER_HPP_
#define SERVER_MESSAGE_HELPER_MESSAGE_CREATERESPONSE_SENDER_HPP_

#include "MessageSender.hpp"

namespace ptree
{
namespace server
{

/** ------------------------------------------------------------------   MessageCreateReponseSender
*
*    HEADER | CREATION_STATUS | OBJECT_PASS
*               (uint8_t)       (uint32)
*/

class MessageCreateResponseSender : public MessageSender
{
public:
    MessageCreateResponseSender(uint32_t transactionId,
        IEndPointPtr endpoint):
        MessageSender(transactionId, endpoint)
    {}

    void send();
    void setResponse(protocol::CreateResponse::Response response);

private:
    protocol::CreateResponse::Response response;
};


} // namespace server
} // namespace ptree

#endif  // SERVER_MESSAGE_HELPER_MESSAGE_CREATERESPONSE_SENDER_HPP_