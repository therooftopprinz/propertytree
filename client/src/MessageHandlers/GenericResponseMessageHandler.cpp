#include "GenericResponseMessageHandler.hpp"

namespace ptree
{
namespace client
{

GenericResponseMessageHandler::
    GenericResponseMessageHandler(TransactionsCV& transactionsCV):
        transactionsCV(transactionsCV), log("GenericResponseMessageHandler")
{
    log << logger::DEBUG << "GenericResponseMessageHandler";
}

GenericResponseMessageHandler::~GenericResponseMessageHandler()
{
    log << logger::DEBUG << "~GenericResponseMessageHandler";
}

void GenericResponseMessageHandler::handle(protocol::MessageHeader& header, Buffer& message)
{
    log << logger::DEBUG << "Receiving msg with tid=" <<  header.transactionId << " type= " << uint16_t(header.type) <<
        " size=" << header.size;
    transactionsCV.notifyTransactionCV(header.transactionId, message);
    log << logger::DEBUG << "handle(): exiting...";
}

} // namespace client
} // namespace ptree
