#ifndef SERVER_MESSAGE_MATCHERS_MATCHER_HPP_
#define SERVER_MESSAGE_MATCHERS_MATCHER_HPP_

#include <common/TestingFramework/EndPointMock.hpp>

namespace ptree
{
namespace server
{

typedef common::MatcherFunctor MatcherFunctor;
typedef common::ActionFunctor ActionFunctor;
typedef common::DefaultAction DefaultAction;

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