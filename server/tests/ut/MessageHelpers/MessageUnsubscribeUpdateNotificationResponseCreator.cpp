#include "MessageUnsubscribeUpdateNotificationResponseCreator.hpp"

namespace ptree
{
namespace server
{

/** ---------------------------------------------- MessageUnsubscribeUpdateNotificationResponseCreator
*   HEADER  | REPSONSE
*            (uint8_t)
*/

Buffer MessageUnsubscribeUpdateNotificationResponseCreator::create()
{
    uint32_t sz = sizeof(protocol::MessageHeader)
        + sizeof(protocol::UnsubscribePropertyUpdateResponse);
    Buffer message(sz);
    createHeader(message.data(), protocol::MessageType::UnsubscribePropertyUpdateResponse, sz);
    protocol::UnsubscribePropertyUpdateResponse
        &subsunr = *(protocol::UnsubscribePropertyUpdateResponse*)
        (message.data() + sizeof(protocol::MessageHeader));
    subsunr.response = response;
    return message;
}

void MessageUnsubscribeUpdateNotificationResponseCreator::setResponse(
    protocol::UnsubscribePropertyUpdateResponse::Response response)
{
    this->response = response;
}

} // namespace server
} // namespace ptree