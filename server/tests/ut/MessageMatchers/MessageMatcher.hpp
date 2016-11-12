#ifndef SERVER_MESSAGE_MATCHERS_MESSAGEMATCHER_HPP_
#define SERVER_MESSAGE_MATCHERS_MESSAGEMATCHER_HPP_

#include "Matcher.hpp"

namespace ptree
{
namespace server
{

class MessageMatcher : public Matcher
{
public:
    MessageMatcher();
    MessageMatcher(Buffer msg);

private:
    bool match(const void *buffer, uint32_t size);

    Buffer msg;
    logger::Logger log;
};


} // namespace core
} // namespace ptree

#endif // SERVER_MESSAGE_MATCHERS_MESSAGEMATCHER_HPP_