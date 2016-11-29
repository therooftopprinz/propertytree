#ifndef SERVER_UTILS_HPP_
#define SERVER_UTILS_HPP_

#include <cstring>
#include <utility>
#include <vector>
#include <memory>

namespace ptree
{

namespace utils
{
void printRaw(const void *begin, uint32_t size);
void printRawAscii(const void *begin, uint32_t size);

std::pair<std::string, std::string> getParentAndChildNames(std::string path);

typedef std::vector<uint8_t> Buffer;
typedef std::shared_ptr<Buffer> BufferPtr;

template<class T>
BufferPtr buildSharedBufferedValue(T value)
{
    BufferPtr data = std::make_shared<Buffer>(sizeof(T));
    if (sizeof(T) == 4 || sizeof(T) == 8)
    {
        *((T*)data->data()) = value;
    }
    else
    {
        std::memcpy(data->data(), &value, sizeof(T));
    }

    return data;
}

template<class T>
Buffer buildBufferedValue(T value)
{
    Buffer data(sizeof(T));

    if (sizeof(T) == 4 || sizeof(T) == 8)
    {
        *((T*)data.data()) = value;
    }
    else
    {
        std::memcpy(data.data(), &value, sizeof(T));
    }

    return data;
}

} // namespace utils
} // namespace ptree 

#endif // SERVER_UTILS_HPP_