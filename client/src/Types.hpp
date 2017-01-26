#include <memory>
#include <vector>
#include <map>

#ifndef CLIENT_TYPES_HPP_
#define CLIENT_TYPES_HPP_

namespace ptree
{
namespace client
{
    typedef std::vector<uint8_t> Buffer;
    typedef std::shared_ptr<Buffer> BufferPtr;

    class PTreeClient;
    typedef std::shared_ptr<PTreeClient> PTreeClientPtr;

    class ValueContainer;
    typedef std::shared_ptr<ValueContainer> ValueContainerPtr;

    template<typename T, typename M = std::mutex>
    struct MutexedObject
    {
        T object;
        M mutex;
    };

} // namespace client
} // namespace ptree

#endif // CLIENT_TYPES_HPP_