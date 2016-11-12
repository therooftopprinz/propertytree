#include "MessageCreator.hpp"

namespace ptree
{
namespace server
{

void MessageCreator::createHeader(void *header, protocol::MessageType type, uint32_t size)
{
    protocol::MessageHeader *h = (protocol::MessageHeader*)(header);
    h->type = type;
    h->size = size;
    h->transactionId = transactionId;
}

} // namespace server
} // namespace ptree