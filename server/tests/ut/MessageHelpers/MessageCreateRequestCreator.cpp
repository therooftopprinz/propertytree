#include "MessageCreateRequestCreator.hpp"

namespace ptree
{
namespace server
{

/** ------------------------------------------------------------------   MessageCreateRequestCreator
*
*    HEADER | VALUE_SIZE | VALUE_TYPE | VALUE | PATH | NULL
*             (uint32_t)    (uint8_t)     []     []     0
*/

Buffer MessageCreateRequestCreator::create()
{
    if (!value)
    {
        value = std::make_shared<server::Buffer>();
    }

    const uint32_t sz = sizeof(protocol::MessageHeader) + 4 + 1 + value->size() + path.length() + 1;
    // log << logger::DEBUG << "MessageCreateRequestCreator::create(): vsize:" << value->size() << " plen:" <<
    //     path.length();

    Buffer message(sz);
    createHeader(message.data(), protocol::MessageType::CreateRequest, sz);
    protocol::CreateRequest &createRequest = *(protocol::CreateRequest*)
        (message.data() + sizeof(protocol::MessageHeader));
    createRequest.size = value->size();
    createRequest.type = type;
    std::memcpy(createRequest.data, value->data(), value->size());
    std::memcpy(createRequest.data + value->size(), path.c_str(), path.length()+1);
    return message;
}

void MessageCreateRequestCreator::setPath(const std::string& path)
{
    this->path = path;
}

void MessageCreateRequestCreator::setType(protocol::PropertyType type)
{
    this->type = type;
}

void MessageCreateRequestCreator::setValue(BufferPtr buffer)
{
    value = buffer;
}

} // namespace server
} // namespace ptree