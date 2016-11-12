#include "MessageSignInRequestCreator.hpp"

namespace ptree
{
namespace server
{

/** -------------------------------------------------------------------- MessageSignInRequestCreator
*
*    HEADER | VERSION
*            (uint32_t)
**/

Buffer MessageSignInRequestCreator::create()
{
    const uint32_t sz = sizeof(protocol::MessageHeader) + sizeof(protocol::SignInRequest);
    Buffer message(sz);
    createHeader(message.data(), protocol::MessageType::SignInRequest, sz);
    protocol::SignInRequest &signin = *(protocol::SignInRequest*)
        (message.data() + sizeof(protocol::MessageHeader));
    signin.version = version;
    signin.updateInterval = updateInterval;
    return message;
}

void MessageSignInRequestCreator::setVersion(uint32_t version)
{
    this->version = version;
}

void MessageSignInRequestCreator::setUpdateInterval(uint32_t rate)
{
    this->updateInterval = rate;
}

} // namespace server
} // namespace ptree