#include "PropertyUpdateNotificationMatcher.hpp"
#include <cstring>
#include <common/src/Utils.hpp>

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
        // log << logger::WARNING << "Not a PropertyUpdateNotification...";
        return false;
    }

    protocol_x::PropertyUpdateNotification  propUpdateNotif;
    protocol_x::BufferView bv(cursor, end);
    propUpdateNotif.parse(bv);

    auto val = ptree->getPropertyByPath<core::Value>(path);

    if (!val)
    {
        return false;
    }

    protocol::Uuid uuid = val->getUuid();

    for(auto& i : propUpdateNotif.propertyUpdateNotifications.get())
    {
        // log << logger::WARNING << "created object: " << *i.path << " with uuid:" << *i.uuid;
        auto val = ptree->getPropertyByUuid<core::Value>(i.uuid);
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

        if (i.uuid == uuid)
        {
            if (valueContainer->size() != ptreeValueSize)
            {
                // log << logger::ERROR << "Sizes are not equal.";
                return false;
            }

            return !std::memcmp(valueContainer->data(), i.data.data(), valueContainer->size());
        }
    }

    return false;
}

} // namespace server
} // namespace ptree