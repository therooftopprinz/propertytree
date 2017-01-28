#ifndef CLIENT_MESSAGEHANDLERS_GENERICRESPONSEMESSAGEHANDLER_HPP_
#define CLIENT_MESSAGEHANDLERS_GENERICRESPONSEMESSAGEHANDLER_HPP_

#include "MessageHandler.hpp"

namespace ptree
{
namespace client
{

struct GenericResponseMessageHandler : public MessageHandler
{
    GenericResponseMessageHandler(TransactionsCV& transactionsCV);
    ~GenericResponseMessageHandler();
    void handle(protocol::MessageHeader& header, Buffer& message);
private:
    TransactionsCV& transactionsCV;
    logger::Logger log;
};


} // namespace client
} // namespace ptree

#endif  // CLIENT_MESSAGEHANDLERS_GENERICRESPONSEMESSAGEHANDLER_HPP_