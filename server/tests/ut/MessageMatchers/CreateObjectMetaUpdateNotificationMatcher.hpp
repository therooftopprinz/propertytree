#ifndef SERVER_MESSAGE_MATCHERS_CREATEOBJECTMETAUPDATENOTIFICATIONMATCHER_HPP_
#define SERVER_MESSAGE_MATCHERS_CREATEOBJECTMETAUPDATENOTIFICATIONMATCHER_HPP_

#include "MetaUpdateNotificationMatcher.hpp"

namespace ptree
{
namespace server
{

class CreateObjectMetaUpdateNotificationMatcher : public MetaUpdateNotificationMatcher
{
public:
    CreateObjectMetaUpdateNotificationMatcher(std::string path);
    uint32_t getUuidOfLastMatched();   
private:
    bool match(const void *buffer, uint32_t size);
    std::string path;
    uint32_t lastMatched = (uint32_t)-1;
};

} // namespace core
} // namespace ptree

#endif // SERVER_MESSAGE_MATCHERS_CREATEOBJECTMETAUPDATENOTIFICATIONMATCHER_HPP_