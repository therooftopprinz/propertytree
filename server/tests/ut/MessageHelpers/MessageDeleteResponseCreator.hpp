#ifndef SERVER_MESSAGE_HELPER_MESSAGE_DELETERESPONSE_CREATOR_HPP_
#define SERVER_MESSAGE_HELPER_MESSAGE_DELETERESPONSE_CREATOR_HPP_

#include "MessageCreator.hpp"

namespace ptree
{
namespace server
{

/** ------------------------------------------------------------------- MessageDeleteResponseCreator
*
*   HEADER  | DELETE_STATUS
*               (uint8_t)
*/

class MessageDeleteResponseCreator : MessageCreator
{
public:
    MessageDeleteResponseCreator(uint32_t transactionId):
        MessageCreator(transactionId)
    {}

    Buffer create();
    void setResponse(protocol::DeleteResponse::Response response);
    
private:
    protocol::DeleteResponse::Response response;
};

} // namespace server
} // namespace ptree

#endif  // SERVER_MESSAGE_HELPER_MESSAGE_DELETERESPONSE_CREATOR_HPP_