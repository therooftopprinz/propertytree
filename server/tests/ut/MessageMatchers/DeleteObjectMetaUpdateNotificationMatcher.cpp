#include "DeleteObjectMetaUpdateNotificationMatcher.hpp"

#include <common/src/Utils.hpp>


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


    log << logger::WARNING << "Matching (" << uuid << ") Delete MetaUpdateNotification...";

    protocol::MetaUpdateNotification deleteMetaNotif;
    protocol::BufferView bv(cursor, end);
    deleteMetaNotif.parse(bv);

    utils::printRaw(cursor, size-sizeof(protocol::MessageHeader));
    utils::printRawAscii(cursor, size-sizeof(protocol::MessageHeader));
    log << logger::WARNING << "deletion len:" << deleteMetaNotif.deletions.get().size();

    for(auto& i : deleteMetaNotif.deletions.get())
    {
        if (uuid == i.uuid)
        {
            log << logger::WARNING << "deleted object with uuid:" << i.uuid;
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