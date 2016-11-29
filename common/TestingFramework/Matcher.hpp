#ifndef SERVER_MESSAGE_MATCHERS_MATCHER_HPP_
#define SERVER_MESSAGE_MATCHERS_MATCHER_HPP_

#include <common/TestingFramework/EndPointMock.hpp>

namespace ptree
{
namespace common
{

class Matcher
{
public:
    MatcherFunctor get();
    virtual ~Matcher() {}
protected:
    virtual bool match(const void *buffer, uint32_t size);
};

} // namespace core
} // namespace ptree

#endif // SERVER_MESSAGE_MATCHERS_MATCHER_HPP_