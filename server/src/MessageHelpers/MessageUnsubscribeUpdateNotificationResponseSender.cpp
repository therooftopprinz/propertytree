#include "MessageUnsubscribeUpdateNotificationResponseSender.hpp"

namespace ptree
{
namespace server
{

/** --------------------------------------------- MessageUnsubscribeUpdateNotificationResponseSender
*   HEADER  | REPSONSE
*            (uint8_t)
*/

void MessageUnsubscribeUpdateNotificationResponseSender::send()
{
    uint32_t sz = sizeof(protocol::MessageHeader)
        + sizeof(protocol::SubscribePropertyUpdateResponse);

    sendHeader(protocol::MessageType::UnsubscribePropertyUpdateResponse, sz);
    endpoint->send(&response, sizeof(response));
}

void MessageUnsubscribeUpdateNotificationResponseSender::setResponse(
    protocol::UnsubscribePropertyUpdateResponse::Response response)
{
    this->response = response;
}


} // namespace server
} // namespace ptree