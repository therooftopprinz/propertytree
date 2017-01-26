#include "GenericResponseMessageHandler.hpp"

namespace ptree
{
namespace client
{

GenericResponseMessageHandler::
    GenericResponseMessageHandler(TransactionsCV& transactionsCV):
        transactionsCV(transactionsCV), log("GenericResponseMessageHandler")
{}

void GenericResponseMessageHandler::handle(protocol::MessageHeader& header, Buffer& message)
{
    log << logger::DEBUG << "Receiving msg with tid=" <<  header.transactionId;
    transactionsCV.notifyTransactionCV(header.transactionId, message);
}

} // namespace client
} // namespace ptree
