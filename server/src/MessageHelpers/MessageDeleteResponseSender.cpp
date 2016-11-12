#include "MessageDeleteResponseSender.hpp"

namespace ptree
{
namespace server
{

/** ------------------------------------------------------------------   MessageDeleteResponseSender
*
*    HEADER | CREATION_STATUS | OBJECT_PASS
*               (uint8_t)       (uint32)
*/

void MessageDeleteResponseSender::send()
{
    uint32_t sz = sizeof(protocol::MessageHeader) + sizeof(protocol::DeleteResponse);

    sendHeader(protocol::MessageType::DeleteResponse, sz);
    endpoint->send(&response, sizeof(response));
}

void MessageDeleteResponseSender::setResponse(protocol::DeleteResponse::Response response)
{
    this->response = response;   
}

} // namespace server
} // namespace ptree