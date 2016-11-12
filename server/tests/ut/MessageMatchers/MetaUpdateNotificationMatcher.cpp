#include "MetaUpdateNotificationMatcher.hpp"

namespace ptree
{
namespace server
{

MetaUpdateNotificationMatcher::MetaUpdateNotificationMatcher() :
    log("MetaUpdateNotificationMatcher")
{
}

void* MetaUpdateNotificationMatcher::getNextEntry(void* start, void* end)
{
    uint8_t *cursor = (uint8_t*)start;
    protocol::MetaUpdateNotification::UpdateType utype =
        *(protocol::MetaUpdateNotification::UpdateType*)(cursor);
    if (utype == protocol::MetaUpdateNotification::UpdateType::CREATE_OBJECT)
    {
        cursor += sizeof(protocol::MetaUpdateNotification::UpdateType) + sizeof(uint32_t);
        if (cursor >= end)
        {
            return cursor;
        }
        while (cursor < end && *cursor)
        {
            cursor++;
        }
        return ++cursor;
    }
    else if (utype == protocol::MetaUpdateNotification::UpdateType::DELETE_OBJECT)
    {
        return cursor + sizeof(protocol::MetaUpdateNotification::UpdateType) + sizeof(uint32_t);
    }
    else
    {
        // log << logger::DEBUG << "Invalid update type!";
        return end;
    }
}


} // namespace server
} // namespace ptree