#ifndef SERVER_MESSAGE_MATCHERS_METAUPDATENOTIFICATION_HPP_
#define SERVER_MESSAGE_MATCHERS_METAUPDATENOTIFICATION_HPP_

#include <common/src/Utils.hpp>
#include "Matcher.hpp"

namespace ptree
{
namespace server
{

class MetaUpdateNotificationMatcher : public Matcher
{
public:
    MetaUpdateNotificationMatcher();
    ~MetaUpdateNotificationMatcher() {}   
protected:
    virtual bool match(const void *buffer, uint32_t size) = 0;
    logger::Logger log;
};

} // namespace core
} // namespace ptree

#endif // SERVER_MESSAGE_MATCHERS_METAUPDATENOTIFICATION_HPP_