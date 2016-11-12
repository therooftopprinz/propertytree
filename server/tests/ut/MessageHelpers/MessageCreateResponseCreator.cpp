#include "MessageCreateResponseCreator.hpp"

namespace ptree
{
namespace server
{

/** ------------------------------------------------------------------   MessageCreateResponseCreator
*
*    HEADER | CREATION_STATUS | OBJECT_PASS
*               (uint8_t)       (uint32)
*/

Buffer MessageCreateResponseCreator::create()
{
    uint32_t sz = sizeof(protocol::MessageHeader) + sizeof(protocol::CreateResponse);
    Buffer message(sz);
    createHeader(message.data(), protocol::MessageType::CreateResponse, sz);
    protocol::CreateResponse &createRsp = *(protocol::CreateResponse*)
        (message.data() + sizeof(protocol::MessageHeader));
    createRsp.response = response;
    createRsp.pass = 0;
    return message;
}

void MessageCreateResponseCreator::setResponse(protocol::CreateResponse::Response response)
{
    this->response = response;   
}

} // namespace server
} // namespace ptree