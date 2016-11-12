#include "MessageSetValueIndicationCreator.hpp"

namespace ptree
{
namespace server
{

/** --------------------------------------------------------------- MessageSetValueIndicationCreator
*   HEADER  |    UUID    |   []   | NULL  
*             (uint32_t)    DATA     0
*/

Buffer MessageSetValueIndicationCreator::create()
{
    uint32_t sz = sizeof(protocol::MessageHeader) + sizeof(uint32_t) + value.size();
    Buffer message(sz);
    createHeader(message.data(), protocol::MessageType::SetValueIndication, sz);
    protocol::SetValueIndication &setval = *(protocol::SetValueIndication*)
        (message.data() + sizeof(protocol::MessageHeader));
    setval.uuid = uuid;
    std::memcpy(setval.data, value.data(), value.size());
    return message;
}

void MessageSetValueIndicationCreator::setUuid(uint32_t uuid)
{
    this->uuid = uuid;
}

void MessageSetValueIndicationCreator::setValue(core::ValueContainer value)
{
    this->value = value;
}

} // namespace server
} // namespace ptree