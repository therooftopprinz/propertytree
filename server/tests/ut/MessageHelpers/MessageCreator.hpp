#ifndef SERVER_MESSAGE_HELPER_MESSAGE_CREATOR_HPP_
#define SERVER_MESSAGE_HELPER_MESSAGE_CREATOR_HPP_

#include <vector>
#include <interface/protocol.hpp>
#include <server/src/ClientServer.hpp>
#include <server/src/Types.hpp>

namespace ptree
{
namespace server
{


class MessageCreator
{
public:
    MessageCreator(uint32_t transactionId):
        transactionId(transactionId),
        log("MessageCreator")
    {}

    void createHeader(void *header, protocol::MessageType type, uint32_t size);

protected:
    uint32_t transactionId;
    logger::Logger log;
};

} // namespace server
} // namespace ptree

#endif  // SERVER_MESSAGE_HELPER_MESSAGE_CREATOR_HPP_