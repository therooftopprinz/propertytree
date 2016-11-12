#include "MessageDeleteResponseCreator.hpp"

namespace ptree
{
namespace server
{

/** ------------------------------------------------------------------- MessageDeleteResponseCreator
*
*   HEADER  | DELETE_STATUS
*               (uint8_t)
*/

Buffer MessageDeleteResponseCreator::create()
{
    uint32_t sz = sizeof(protocol::MessageHeader) + sizeof(protocol::DeleteResponse);
    Buffer message(sz);
    createHeader(message.data(), protocol::MessageType::DeleteResponse, sz);
    protocol::DeleteResponse &deleteResponse = *(protocol::DeleteResponse*)
        (message.data() + sizeof(protocol::MessageHeader));
    deleteResponse.response = response;
    return message;
}

void MessageDeleteResponseCreator::setResponse(protocol::DeleteResponse::Response response)
{
    this->response = response;
}

} // namespace server
} // namespace ptree