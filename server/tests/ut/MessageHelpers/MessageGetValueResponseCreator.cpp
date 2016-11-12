#include "MessageGetValueResponseCreator.hpp"

namespace ptree
{
namespace server
{

/** ------------------------------------------------------------------- MessageGetValueResponseCreator
*
*   HEADER  | DATA
*              []
*/

Buffer MessageGetValueResponseCreator::create()
{
    uint32_t vsize = 0;
    
    if (value)
    {
        vsize = value->size();
    }

    uint32_t sz = sizeof(protocol::MessageHeader) + vsize;
    Buffer message(sz);
    createHeader(message.data(), protocol::MessageType::GetValueResponse, sz);
    protocol::GetValueResponse &getValueResponse = *(protocol::GetValueResponse*)
        (message.data() + sizeof(protocol::MessageHeader));

    std::memcpy(&getValueResponse, value->data(), vsize);

    return message;
}

void MessageGetValueResponseCreator::setValue(BufferPtr value)
{
    this->value = value;
}

} // namespace server
} // namespace ptree