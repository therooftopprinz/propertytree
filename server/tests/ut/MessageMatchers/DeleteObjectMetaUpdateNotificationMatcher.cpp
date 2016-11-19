#include "DeleteObjectMetaUpdateNotificationMatcher.hpp"

namespace ptree
{
namespace server
{



DeleteObjectMetaUpdateNotificationMatcher::DeleteObjectMetaUpdateNotificationMatcher(uint32_t uuid):
    uuid(uuid)
{

}

bool DeleteObjectMetaUpdateNotificationMatcher::match(const void *buffer, uint32_t size)
{
    uint8_t *cursor = (uint8_t*)buffer + sizeof(protocol::MessageHeader);
    uint8_t *end = (uint8_t*)buffer + size;

    protocol::MessageHeader *head = (protocol::MessageHeader*)buffer;
    if (head->type != protocol::MessageType::MetaUpdateNotification)
    {
        // log << logger::WARNING << "Not a MetaUpdateNotification...";
        return false;
    }


    // log << logger::WARNING << "Matching Create MetaUpdateNotification...";

    protocol::MetaUpdateNotification  createMetaNotif;
    protocol::Decoder de(cursor, end);
    createMetaNotif << de;

    for(auto& i : *createMetaNotif.deletions)
    {
        log << logger::WARNING << "created object with uuid:" << *i.uuid;
        if (uuid == *i.uuid)
        {
            return true;
        }
    }

    return false;
}

void DeleteObjectMetaUpdateNotificationMatcher::setUuid(uint32_t uuid)
{
    this->uuid = uuid;
}


} // namespace server
} // namespace ptree