#include "MessageSubscribeUpdateNotificationResponseCreator.hpp"

namespace ptree
{
namespace server
{

/** ---------------------------------------------- MessageSubscribeUpdateNotificationResponseCreator
*   HEADER  | REPSONSE
*            (uint8_t)
*/

Buffer MessageSubscribeUpdateNotificationResponseCreator::create()
{
    uint32_t sz = sizeof(protocol::MessageHeader)
        + sizeof(protocol::SubscribePropertyUpdateResponse);
    Buffer message(sz);
    createHeader(message.data(), protocol::MessageType::SubscribePropertyUpdateResponse, sz);
    protocol::SubscribePropertyUpdateResponse
        &subsunr = *(protocol::SubscribePropertyUpdateResponse*)
        (message.data() + sizeof(protocol::MessageHeader));
    subsunr.response = response;
    return message;
}

void MessageSubscribeUpdateNotificationResponseCreator::setResponse(
    protocol::SubscribePropertyUpdateResponse::Response response)
{
    this->response = response;
}

} // namespace server
} // namespace ptree