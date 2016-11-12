#include "MessageGetValueResponseSender.hpp"

namespace ptree
{
namespace server
{

/** ------------------------------------------------------------------- MessageSignInResponseSender
*
*    HEADER | VERSION
*            (uint32_t)
**/

void MessageGetValueResponseSender::send()
{
    uint32_t vsize = 0;

    if (value)
    {
        vsize = value->getValue().size();
    }

    const uint32_t sz = sizeof(protocol::MessageHeader) + vsize;
    sendHeader(protocol::MessageType::GetValueResponse, sz);

    if(vsize)
    {
        endpoint->send(value->getValue().data(), vsize);
    }

    log << logger::DEBUG << "MessageGetValueResponse sent!! size:" << vsize;
}

void MessageGetValueResponseSender::setValue(core::ValuePtr value)
{
    this->value = value;
}

} // namespace server
} // namespace ptree