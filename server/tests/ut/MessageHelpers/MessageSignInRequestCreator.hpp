#ifndef SERVER_MESSAGE_HELPER_MESSAGE_SIGNINREQUEST_CREATOR_HPP_
#define SERVER_MESSAGE_HELPER_MESSAGE_SIGNINREQUEST_CREATOR_HPP_

#include "MessageCreator.hpp"

namespace ptree
{
namespace server
{

/** -------------------------------------------------------------------- MessageSignInRequestCreator
*
*    HEADER | VERSION
*            (uint32_t)
**/

class MessageSignInRequestCreator : public MessageCreator
{
public:
    MessageSignInRequestCreator(uint32_t transactionId):
        MessageCreator(transactionId),
        version(0),
        updateInterval(100)
    {}

    Buffer create();
    void setVersion(uint32_t version);
    void setUpdateInterval(uint32_t rate);

private:
    uint32_t version;
    uint32_t updateInterval;
};

} // namespace server
} // namespace ptree

#endif  // SERVER_MESSAGE_HELPER_MESSAGE_SIGNINREQUEST_CREATOR_HPP_