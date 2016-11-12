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
    // log << logger::DEBUG << "Trying full match with:";
    // utils::printRaw(msg.data(), msg.size());
    if(msg.size()!=size)
    {            
        // log << logger::DEBUG << "Inequal sizes!";
        return false;
    }

    if (!std::memcmp(buffer, msg.data(), size))
    {
        // log << logger::DEBUG << "Send: sent and expected. ";
        return true;
    }

    // log << logger::DEBUG << "Message didn't match. ";
    return false;
}

} // namespace server
} // namespace ptree