#ifndef SERVER_MESSAGE_MATCHERS_PROPERTYUPDATENOTIFICATION_HPP_
#define SERVER_MESSAGE_MATCHERS_PROPERTYUPDATENOTIFICATION_HPP_

#include <cstring>
#include <server/src/Utils.hpp>
#include <server/src/PTree.hpp>
#include <server/src/Types.hpp>

#include "Matcher.hpp"

namespace ptree
{
namespace server
{

class PropertyUpdateNotificationMatcher : public Matcher
{
public:
    PropertyUpdateNotificationMatcher(std::string path,
        BufferPtr valueContainer, core::PTreePtr ptree);
    ~PropertyUpdateNotificationMatcher() {}   
    bool match(const void *buffer, uint32_t size);
protected:
    core::PTreePtr ptree;
    BufferPtr valueContainer;
    std::string path;
    logger::Logger log;
};

} // namespace core
} // namespace ptree

#endif // SERVER_MESSAGE_MATCHERS_PROPERTYUPDATENOTIFICATION_HPP_