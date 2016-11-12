#ifndef SERVER_MESSAGE_HELPER_MESSAGE_SIGNINRESPONSE_SENDER_HPP_
#define SERVER_MESSAGE_HELPER_MESSAGE_SIGNINRESPONSE_SENDER_HPP_

#include "MessageSender.hpp"

namespace ptree
{
namespace server
{

/** ------------------------------------------------------------------- MessageSignInResponseSender
*
*    HEADER | VERSION
*            (uint32_t)
**/

class MessageSignInResponseSender : public MessageSender
{
public:
    MessageSignInResponseSender(uint32_t transactionId,
        IEndPointPtr endpoint):
        MessageSender(transactionId, endpoint),
        version(0)
    {}

    void send();
    void setVersion(uint32_t version);

private:
    uint32_t version;
};

} // namespace server
} // namespace ptree

#endif  // SERVER_MESSAGE_HELPER_MESSAGE_SIGNINRESPONSE_SENDER_HPP_