#include "MessageHandler.hpp"

namespace ptree
{
namespace server
{

void MessageHandler::handle(protocol::MessageHeaderPtr header, BufferPtr message)
{
}

Buffer MessageHandler::createHeader(protocol::MessageType type, uint32_t size, uint32_t transactionId)
{
    Buffer header(sizeof(protocol::MessageHeader));
    protocol::MessageHeader& headerRaw = *((protocol::MessageHeader*)header.data());
    headerRaw.type = type;
    headerRaw.size = size;
    headerRaw.transactionId = transactionId;
    return header;
}

} // namespace server
} // namespace ptree