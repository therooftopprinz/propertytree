#include "MessageDeleteRequestCreator.hpp"

namespace ptree
{
namespace server
{

/** -------------------------------------------------------------------- MessageDeleteRequestCreator
*
*   HEADER  | PATH | NULL
*              []     0
*/

Buffer MessageDeleteRequestCreator::create()
{
    uint32_t sz = sizeof(protocol::MessageHeader) + path.length() + 1 /*null*/;
    Buffer message(sz);
    createHeader(message.data(), protocol::MessageType::DeleteRequest, sz);
    std::memcpy(message.data()+ sizeof(protocol::MessageHeader), path.c_str(), path.size()+1);
    return message;
}

void MessageDeleteRequestCreator::setPath(std::string path)
{
    this->path = path;
}

} // namespace server
} // namespace ptree