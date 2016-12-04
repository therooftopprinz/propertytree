#include "MessageHandler.hpp"

namespace ptree
{
namespace client
{

void MessageHandler::handle(protocol::MessageHeaderPtr header, BufferPtr message)
{
}

Buffer MessageHandler::createHeader(protocol::MessageType type, uint32_t payloadSize, uint32_t transactionId)
{
    Buffer header(sizeof(protocol::MessageHeader));
    protocol::MessageHeader& headerRaw = *((protocol::MessageHeader*)header.data());
    headerRaw.type = type;
    headerRaw.size = payloadSize+sizeof(protocol::MessageHeader);
    headerRaw.transactionId = transactionId;
    return header;
}

} // namespace client
} // namespace ptree