#include "CreateObjectMetaUpdateNotificationMatcher.hpp"

#include <common/src/Utils.hpp>

namespace ptree
{
namespace server
{


CreateObjectMetaUpdateNotificationMatcher::CreateObjectMetaUpdateNotificationMatcher(
    std::string path) :
    path(path)
{

}

uint32_t CreateObjectMetaUpdateNotificationMatcher::getUuidOfLastMatched()
{
    return lastMatched;
}

bool CreateObjectMetaUpdateNotificationMatcher::match(const void *buffer, uint32_t size)
{
    uint8_t *cursor = (uint8_t*)buffer + sizeof(protocol::MessageHeader);
    uint8_t *end = (uint8_t*)buffer + size;

    protocol::MessageHeader *head = (protocol::MessageHeader*)buffer;
    if (head->type != protocol::MessageType::MetaUpdateNotification)
    {
        // log << logger::WARNING << "Not a MetaUpdateNotification...";
        return false;
    }


    log << logger::WARNING << "Matching (" << path << ") Create MetaUpdateNotification...";

    protocol_x::MetaUpdateNotification createMetaNotif;
    protocol_x::BufferView bv(cursor, end);
    createMetaNotif.parse(bv);

    utils::printRaw(cursor, size-sizeof(protocol::MessageHeader));
    utils::printRawAscii(cursor, size-sizeof(protocol::MessageHeader));
    log << logger::WARNING << "creation len: " << createMetaNotif.creations.get().size();

    for(auto& i : createMetaNotif.creations.get())
    {
        if (path == i.path)
        {
            log << logger::WARNING << "created object: " << i.path << " with uuid:" << i.uuid;
            lastMatched = i.uuid;
            return true;
        }
    }

    return false;
}

} // namespace server
} // namespace ptree