#ifndef SERVER_MESSAGE_MATCHERS_MATCHER_HPP_
#define SERVER_MESSAGE_MATCHERS_MATCHER_HPP_

#include <server/tests/ut/framework/EndPointMock.hpp>

namespace ptree
{
namespace server
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