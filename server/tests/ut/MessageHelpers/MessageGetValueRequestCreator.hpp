#ifndef SERVER_MESSAGE_HELPER_MESSAGE_GETVALUEREQUEST_CREATOR_HPP_
#define SERVER_MESSAGE_HELPER_MESSAGE_GETVALUEREQUEST_CREATOR_HPP_

#include "MessageCreator.hpp"

namespace ptree
{
namespace server
{

/** ------------------------------------------------------------------- MessageDeleteResponseCreator
*
*   HEADER  |    UUID
*             (uint32_t)
*/

class MessageGetValueRequestCreator : MessageCreator
{
public:
    MessageGetValueRequestCreator(uint32_t transactionId):
        MessageCreator(transactionId)
    {}

    Buffer create();
    void setUuid(protocol::Uuid uuid);
    
private:
    protocol::Uuid uuid;;
};

} // namespace server
} // namespace ptree

#endif  // SERVER_MESSAGE_HELPER_MESSAGE_DELETERESPONSE_CREATOR_HPP_