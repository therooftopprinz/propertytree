#include "MessageCreateResponseSender.hpp"

namespace ptree
{
namespace server
{

/** ------------------------------------------------------------------   MessageCreateReponseSender
*
*    HEADER | CREATION_STATUS | OBJECT_PASS
*               (uint8_t)       (uint32)
*/

void MessageCreateResponseSender::send()
{
    uint32_t sz = sizeof(protocol::MessageHeader) + sizeof(protocol::CreateResponse);
    uint32_t pass = 0;

    sendHeader(protocol::MessageType::CreateResponse, sz);
    
    endpoint->send(&response, sizeof(response));
    endpoint->send(&pass, sizeof(pass));
}

void MessageCreateResponseSender::setResponse(protocol::CreateResponse::Response response)
{
    this->response = response;   
}

} // namespace server
} // namespace ptree