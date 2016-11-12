#include "MessageSubscribeUpdateNotificationResponseSender.hpp"

namespace ptree
{
namespace server
{

/** ---------------------------------------------- MessageSubscribeUpdateNotificationResponseSender
*   HEADER  | REPSONSE
*            (uint8_t)
*/

void MessageSubscribeUpdateNotificationResponseSender::send()
{
    uint32_t sz = sizeof(protocol::MessageHeader)
        + sizeof(protocol::SubscribePropertyUpdateResponse);

    sendHeader(protocol::MessageType::SubscribePropertyUpdateResponse, sz);
    endpoint->send(&response, sizeof(response));
}

void MessageSubscribeUpdateNotificationResponseSender::setResponse(
    protocol::SubscribePropertyUpdateResponse::Response response)
{
    this->response = response;
}


} // namespace server
} // namespace ptree