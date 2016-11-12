#include "MessagePropertyUpdateNotificationSender.hpp"

namespace ptree
{
namespace server
{

/** ------------------------------------------------------- MessagePropertyUpdateNotificationSender
*   HEADER  | (   UUID    |    SIZE    |  DATA   )
*               (uint8_t)   (uint32_t)     []
*/

void MessagePropertyUpdateNotificationSender::send()
{
    uint32_t sz = sizeof(protocol::MessageHeader) + getMessageSize();

    protocol::MessageHeader header;
    header.type = protocol::MessageType::PropertyUpdateNotification;
    header.size = sz;
    header.transactionId = transactionId;

    endpoint->send(&header, sizeof(protocol::MessageHeader));

    for (const auto& update : valueUpdate)
    {
        uint32_t size = update->getValue().size();
        uint32_t uuid = update->getUuid();
        log << logger::DEBUG << "Creating creation entry for : " << uuid;

        endpoint->send(&uuid, sizeof(uint32_t));
        // endpoint->send(&size, sizeof(uint32_t));
        endpoint->send(update->getValue().data(), size);
    }
}

uint32_t MessagePropertyUpdateNotificationSender::getMessageSize()
{
    uint32_t size = 0;
    for (const auto& value : valueUpdate)
    {
        // size += (sizeof(uint32_t) + sizeof(uint32_t) + value->getValue().size());
        size += (sizeof(uint32_t) + value->getValue().size());
        /**     uuid + data **/
    }

    return size;

}

} // namespace server
} // namespace ptree