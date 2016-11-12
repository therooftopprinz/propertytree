#ifndef SERVER_MESSAGE_MATCHERS_DELETEOBJECTMETAUPDATENOTIFICATION_HPP_
#define SERVER_MESSAGE_MATCHERS_DELETEOBJECTMETAUPDATENOTIFICATION_HPP_

#include "MetaUpdateNotificationMatcher.hpp"

namespace ptree
{
namespace server
{


class DeleteObjectMetaUpdateNotificationMatcher : public MetaUpdateNotificationMatcher
{
public:
    DeleteObjectMetaUpdateNotificationMatcher() {}
    DeleteObjectMetaUpdateNotificationMatcher(uint32_t uuid);
    void setUuid(uint32_t uuid);
private:
    bool match(const void *buffer, uint32_t size);
    uint32_t uuid = (uint32_t)-1;
};


} // namespace core
} // namespace ptree

#endif // SERVER_MESSAGE_MATCHERS_DELETEOBJECTMETAUPDATENOTIFICATION_HPP_