#include "MessageMetaUpdateNotificationSender.hpp"

namespace ptree
{
namespace server
{

/** ---------------------------------------------------------   MessageMetaUpdateNotificationSender
*
*   HEADER | ( CREATE_OBJECT |     UUID     | PROPERTY_TYPE | PATH | NULL )...
*                (uint8_t)      (uint32_t)      (uint8_t)      []     0
*          , ( DELETE_OBJECT |    UUID     )...
*                (uint8_t)       (uint32_t)
*/

void MessageMetaUpdateNotificationSender::send()
{
    uint32_t sz = sizeof(protocol::MessageHeader) + getMessageSize();
    sendHeader(protocol::MessageType::MetaUpdateNotification, sz);

    for (const auto& update : uuidActionTypeAndPathMap)
    {
        if (update.second.utype == protocol::MetaUpdateNotification::UpdateType::CREATE_OBJECT)
        {
            log << logger::DEBUG << "Creating creation entry for : " << update.second.path;

            protocol::MetaUpdateNotification::UpdateType updateType =
                protocol::MetaUpdateNotification::UpdateType::CREATE_OBJECT;
            endpoint->send(&updateType, sizeof(protocol::MetaUpdateNotification::UpdateType));
            
            uint32_t uuid = update.first;
            endpoint->send(&uuid, sizeof(uuid));

            protocol::PropertyType ptype = update.second.ptype;
            endpoint->send(&ptype, sizeof(protocol::PropertyType));

            endpoint->send(update.second.path.c_str(), update.second.path.length()+1);
        }
        else if (update.second.utype == protocol::MetaUpdateNotification::UpdateType::DELETE_OBJECT)
        {
            log << logger::DEBUG << "Creating deletion entry for : " << update.first;
            protocol::MetaUpdateNotification::UpdateType updateType =
                protocol::MetaUpdateNotification::UpdateType::DELETE_OBJECT;
            endpoint->send(&updateType, sizeof(protocol::MetaUpdateNotification::UpdateType));
            
            uint32_t uuid = update.first;
            endpoint->send(&uuid, sizeof(uuid));
        }
    }
}

uint32_t MessageMetaUpdateNotificationSender::getMessageSize()
{
    uint32_t size = 0;
    for (const auto& update : uuidActionTypeAndPathMap)
    {
        if (update.second.utype == protocol::MetaUpdateNotification::UpdateType::CREATE_OBJECT)
        {
            size += (1 + 4 + 1 + update.second.path.size() + 1);
            /**     update type + uuid + property type + path + 0 **/
        }
        
        if (update.second.utype == protocol::MetaUpdateNotification::UpdateType::DELETE_OBJECT)
        {
            size += (1 + 4);
            /**     update type + uuid **/
        }
    }

    return size;
}
} // namespace server
} // namespace ptree