#include "MessageSignInResponseSender.hpp"

namespace ptree
{
namespace server
{

/** ------------------------------------------------------------------- MessageSignInResponseSender
*
*    HEADER | VERSION
*            (uint32_t)
**/

void MessageSignInResponseSender::send()
{
    const uint32_t sz = sizeof(protocol::MessageHeader) + sizeof(protocol::SignInResponse);
    sendHeader(protocol::MessageType::SignInResponse, sz);
    endpoint->send(&version, sizeof(version));
    log << logger::DEBUG << "MessageSignInResponse sent!! size:" << sz;
}

void MessageSignInResponseSender::setVersion(uint32_t version)
{
    this->version = version;
}


} // namespace server
} // namespace ptree