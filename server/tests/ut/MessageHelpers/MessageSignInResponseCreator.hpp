#ifndef SERVER_MESSAGE_HELPER_MESSAGE_SIGNINRESPONSE_CREATOR_HPP_
#define SERVER_MESSAGE_HELPER_MESSAGE_SIGNINRESPONSE_CREATOR_HPP_

#include "MessageCreator.hpp"

namespace ptree
{
namespace server
{

/** ------------------------------------------------------------------- MessageSignInResponseCreator
*
*    HEADER | VERSION
*            (uint32_t)
**/

class MessageSignInResponseCreator : MessageCreator
{
public:
    MessageSignInResponseCreator(uint32_t transactionId):
        MessageCreator(transactionId),
        version(0)
    {}

    Buffer create();
    void setVersion(uint32_t version);

private:
    uint32_t version;
};

} // namespace server
} // namespace ptree

#endif  // SERVER_MESSAGE_HELPER_MESSAGE_SIGNINRESPONSE_CREATOR_HPP_