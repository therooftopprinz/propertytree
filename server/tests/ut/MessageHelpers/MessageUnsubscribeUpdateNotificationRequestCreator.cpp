#include "MessageUnsubscribeUpdateNotificationRequestCreator.hpp"

namespace ptree
{
namespace server
{

/** ----------------------------------------------- MessageUnsubscribeUpdateNotificationRequestCreator
*   HEADER  |    UUID  
*             (uint32_t)
*/

Buffer MessageUnsubscribeUpdateNotificationRequestCreator::create()
{
    uint32_t sz = sizeof(protocol::MessageHeader) + sizeof(uint32_t);
    Buffer message(sz);
    createHeader(message.data(), protocol::MessageType::UnsubscribePropertyUpdateRequest, sz);
    protocol::UnsubscribePropertyUpdateRequest &setval = *(protocol::UnsubscribePropertyUpdateRequest*)
        (message.data() + sizeof(protocol::MessageHeader));
    setval.uuid = uuid;
    return message;
}

void MessageUnsubscribeUpdateNotificationRequestCreator::setUuid(uint32_t uuid)
{
    this->uuid = uuid;
}

} // namespace server
} // namespace ptree