#include "MessageSignInResponseCreator.hpp"

namespace ptree
{
namespace server
{

/** ------------------------------------------------------------------- MessageSignInResponseCreator
*
*    HEADER | VERSION
*            (uint32_t)
**/

Buffer MessageSignInResponseCreator::create()
{
    const uint32_t sz = sizeof(protocol::MessageHeader) + sizeof(protocol::SignInResponse);
    Buffer message(sz);
    createHeader(message.data(), protocol::MessageType::SignInResponse, sz);
    protocol::SignInResponse &signin = *(protocol::SignInResponse*)
        (message.data() + sizeof(protocol::MessageHeader));
    signin.version = version;
    log << logger::DEBUG << "MessageSignInResponseCreator::create size:" << message.size();
    return message;
}

void MessageSignInResponseCreator::setVersion(uint32_t version)
{
    this->version = version;
}

} // namespace server
} // namespace ptree