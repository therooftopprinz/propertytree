#include "MessageSubscribeUpdateNotificationRequestCreator.hpp"

namespace ptree
{
namespace server
{

/** ----------------------------------------------- MessageSubscribeUpdateNotificationRequestCreator
*   HEADER  |    UUID  
*             (uint32_t)
*/

Buffer MessageSubscribeUpdateNotificationRequestCreator::create()
{
    uint32_t sz = sizeof(protocol::MessageHeader) + sizeof(uint32_t);
    Buffer message(sz);
    createHeader(message.data(), protocol::MessageType::SubscribePropertyUpdateRequest, sz);
    protocol::SubscribePropertyUpdateRequest &setval = *(protocol::SubscribePropertyUpdateRequest*)
        (message.data() + sizeof(protocol::MessageHeader));
    setval.uuid = uuid;
    return message;
}

void MessageSubscribeUpdateNotificationRequestCreator::setUuid(uint32_t uuid)
{
    this->uuid = uuid;
}

} // namespace server
} // namespace ptree