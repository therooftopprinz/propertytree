#include "PropertyUpdateNotificationMatcher.hpp"
#include <cstring>
#include <server/src/Utils.hpp>

namespace ptree
{
namespace server
{


PropertyUpdateNotificationMatcher::PropertyUpdateNotificationMatcher(std::string path,
    BufferPtr valueContainer, core::PTreePtr ptree) :
        ptree(ptree),
        valueContainer(valueContainer),
        path(path),
        log("MetaUpdateNotificationMatcher")
{
}

bool PropertyUpdateNotificationMatcher::match(const void *buffer, uint32_t size)
{
    uint8_t *cursor = (uint8_t*)buffer + sizeof(protocol::MessageHeader);
    uint8_t *end = (uint8_t*)buffer + size;

    protocol::MessageHeader *head = (protocol::MessageHeader*)buffer;
    if (head->type != protocol::MessageType::PropertyUpdateNotification)
    {
        // log << logger::WARNING << "Message is not a PropertyUpdateNotification.";
        return false;
    }

    auto val = ptree->getPropertyByPath<core::Value>(path);

    if (!val)
    {
        return false;
    }

    protocol::Uuid uuid = val->getUuid();

    while (cursor < end)
    {
        const protocol::Uuid& id = *((protocol::Uuid*)cursor);
        // log << logger::DEBUG << "Item at: " << id << " with id: " << id;

        auto val = ptree->getPropertyByUuid<core::Value>(id);
        uint32_t ptreeValueSize = 0;

        if (val)
        {
            ptreeValueSize = val->getValue().size();
            // log << logger::DEBUG << "and size: " << ptreeValueSize;
        }
        else
        {
            // log << logger::ERROR << "The object: " << path << "is not on the model nor Value!";
            return false;
        }

        if (id == uuid)
        {
            if (valueContainer->size() != ptreeValueSize)
            {
                // log << logger::ERROR << "Sizes are not equal.";
                return false;
            }

            return !std::memcmp(valueContainer->data(),
                cursor+sizeof(protocol::Uuid), valueContainer->size());
        }
        // else
        // {
        //     log << logger::DEBUG << "This is not the correct Uuid.";
        // }

        cursor += sizeof(protocol::Uuid) + ptreeValueSize;
    }

    // log << logger::DEBUG << "Not matched!";
    return false;
}

} // namespace server
} // namespace ptree