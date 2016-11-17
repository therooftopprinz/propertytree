#include "CreateObjectMetaUpdateNotificationMatcher.hpp"

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
        log << logger::WARNING << "Not a MetaUpdateNotification...";
        return false;
    }


    log << logger::WARNING << "Matching Create MetaUpdateNotification...";

    protocol::MetaUpdateNotification  createMetaNotif;
    protocol::Decoder de(cursor, end);
    createMetaNotif << de;

    for(auto& i : *createMetaNotif.creations)
    {
        log << logger::WARNING << "created object: " << *i.path << " with uuid:" << *i.uuid;
        if (path == *i.path)
        {
            return true;
        }
    }

    return false;
}

} // namespace server
} // namespace ptree