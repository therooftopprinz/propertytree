#ifndef SERVER_MESSAGE_HELPER_MESSAGE_SENDER_HPP_
#define SERVER_MESSAGE_HELPER_MESSAGE_SENDER_HPP_

#include <server/src/ClientServer.hpp>

namespace ptree
{
namespace server
{


class MessageSender
{
public:
    MessageSender(uint32_t transactionId, IEndPointPtr endpoint):
        transactionId(transactionId),
        endpoint(endpoint),
        log("MessageSender")
    {}

    void sendHeader(protocol::MessageType type, uint32_t size);
    virtual void send() = 0;

protected:
    uint32_t transactionId;
    IEndPointPtr endpoint;
    logger::Logger log;
};

} // namespace server
} // namespace ptree

#endif  // SERVER_MESSAGE_HELPER_MESSAGE_SENDER_HPP_