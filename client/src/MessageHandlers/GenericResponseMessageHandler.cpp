#include "GenericResponseMessageHandler.hpp"

namespace ptree
{
namespace client
{

GenericResponseMessageHandler::
    GenericResponseMessageHandler(PTreeClient& pc, IEndPoint& ep):
        MessageHandler(pc, ep)
{}

void GenericResponseMessageHandler::handle(protocol::MessageHeaderPtr header, BufferPtr message)
{
    ptreeClient.notifyTransactionCV(header->transactionId, message);
}

} // namespace client
} // namespace ptree
