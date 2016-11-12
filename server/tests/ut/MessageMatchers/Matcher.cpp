#include "Matcher.hpp"

namespace ptree
{
namespace server
{

bool Matcher::match(const void *buffer, uint32_t size)
{
    return false;
}

MatcherFunctor Matcher::get()
{
    using std::placeholders::_1;
    using std::placeholders::_2;
    return std::bind(&Matcher::match, this, _1, _2);
}

} // namespace server
} // namespace ptree