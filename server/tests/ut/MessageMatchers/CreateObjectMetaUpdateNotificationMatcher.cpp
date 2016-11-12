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
        // log << logger::WARNING << "Message is not a MetaUpdate.";
        return false;
    }

    // log << logger::DEBUG << "Request match for object creation of: " << path;
    // log << logger::DEBUG << "msg: " << (void*)buffer;
    // log << logger::DEBUG << "begin: " << (void*)cursor;
    // log << logger::DEBUG << "end " << (void*)end;

    while (cursor < end)
    {
        // log << logger::DEBUG << "Item at: " << (void*)cursor;
        // utils::printRawAscii(cursor, end-cursor);

        protocol::MetaUpdateNotification::UpdateType utype =
            *(protocol::MetaUpdateNotification::UpdateType*)(cursor);

        // if (utype == protocol::MetaUpdateNotification::UpdateType::CREATE_OBJECT)
        // {
        //     log << logger::DEBUG << "Item is create.";
        //     log << logger::DEBUG << "Path: " <<
        //         (const char *)(cursor + sizeof(protocol::MetaUpdateNotification::UpdateType) +
        //         sizeof(uint32_t) + sizeof(protocol::PropertyType));
        // }
        // else if (utype == protocol::MetaUpdateNotification::UpdateType::DELETE_OBJECT)
        // {
        //     log << logger::DEBUG << "Item is delete.";
        // }
        // else
        // {
        //     log << logger::WARNING << "Uknown update type!!!!";
        // }

        if (utype == protocol::MetaUpdateNotification::UpdateType::CREATE_OBJECT && 
            path ==  (const char *)(cursor + sizeof(protocol::MetaUpdateNotification::UpdateType) +
                sizeof(uint32_t) + sizeof(protocol::PropertyType)))
        {
            lastMatched = *(uint32_t*)(cursor + sizeof(protocol::MetaUpdateNotification::UpdateType));
            // log << logger::DEBUG << "Matched! uuid: " << lastMatched;
            return true;
        }

        
        cursor = (uint8_t*)getNextEntry(cursor, end);
    }

    // log << logger::DEBUG << "Not matched!";
    return false;
}

} // namespace server
} // namespace ptree