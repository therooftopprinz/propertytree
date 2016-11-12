#ifndef SERVER_MESSAGE_HELPER_MESSAGE_CREATERESPONSE_CREATOR_HPP_
#define SERVER_MESSAGE_HELPER_MESSAGE_CREATERESPONSE_CREATOR_HPP_

#include "MessageCreator.hpp"

namespace ptree
{
namespace server
{

/** ------------------------------------------------------------------   MessageCreateResponseCreator
*
*    HEADER | CREATION_STATUS | OBJECT_PASS
*               (uint8_t)       (uint32)
*/

class MessageCreateResponseCreator : MessageCreator
{
public:
    MessageCreateResponseCreator(uint32_t transactionId):
        MessageCreator(transactionId)
    {}

    Buffer create();
    void setResponse(protocol::CreateResponse::Response response);

private:
    protocol::CreateResponse::Response response;
};


} // namespace server
} // namespace ptree

#endif  // SERVER_MESSAGE_HELPER_MESSAGE_CREATERESPONSE_CREATOR_HPP_