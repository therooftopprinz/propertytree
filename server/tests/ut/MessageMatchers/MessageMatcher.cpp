#include "MessageMatcher.hpp"
#include <server/src/Utils.hpp>
namespace ptree
{
namespace server
{

MessageMatcher::MessageMatcher() :
        log("MessageMatcher")
{

}

MessageMatcher::MessageMatcher(const Buffer msg) :
        msg(msg),
        log("MessageMatcher")
{

}

bool MessageMatcher::match(const void *buffer, uint32_t size)
{
    // log << logger::WARNING << "Trying full match with:";
    utils::printRaw(msg.data(), msg.size());
    if(msg.size()!=size)
    {            
        // log << logger::WARNING << "Inequal sizes!";
        return false;
    }

    if (!std::memcmp(buffer, msg.data(), size))
    {
        // utils::printRaw(msg.data(), msg.size());
        log << logger::WARNING << "Send: sent and expected. ";
        return true;
    }

    // log << logger::WARNING << "Message didn't match. ";
    return false;
}

} // namespace server
} // namespace ptree