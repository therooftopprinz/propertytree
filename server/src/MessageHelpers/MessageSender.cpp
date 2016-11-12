#include "MessageSender.hpp"

namespace ptree
{
namespace server
{

void MessageSender::sendHeader(protocol::MessageType type, uint32_t size)
{
    protocol::MessageHeader header;
    header.type = type;
    header.size = size;
    header.transactionId = transactionId;

    endpoint->send(&header, sizeof(protocol::MessageHeader));
}

}
}