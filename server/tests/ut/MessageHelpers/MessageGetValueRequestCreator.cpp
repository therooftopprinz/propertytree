#include "MessageGetValueRequestCreator.hpp"

namespace ptree
{
namespace server
{

/** ------------------------------------------------------------------- MessageDeleteResponseCreator
*
*   HEADER  |    UUID
*             (uint32_t)
*/

Buffer MessageGetValueRequestCreator::create()
{
    uint32_t sz = sizeof(protocol::MessageHeader) + sizeof(protocol::Uuid);
    Buffer message(sz);
    createHeader(message.data(), protocol::MessageType::GetValueRequest, sz);
    protocol::GetValueRequest &setval = *(protocol::GetValueRequest*)
        (message.data() + sizeof(protocol::MessageHeader));
    setval.uuid = uuid;
    return message;
}

void MessageGetValueRequestCreator::setUuid(uint32_t uuid)
{
    this->uuid = uuid;
}

} // namespace server
} // namespace ptree