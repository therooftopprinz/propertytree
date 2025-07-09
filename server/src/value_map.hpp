#ifndef __VALUE_MAP_HPP__
#define __VALUE_MAP_HPP__

#include <unordered_set>
#include <optional>
#include <vector>
#include <memory>
#include <map>

#include <bfc/socket.hpp>

namespace propertytree
{

struct client_context
{
    virtual ~client_context() = default;
    virtual int send(const bfc::const_buffer_view&) = 0;
};

using client_context_ptr = std::shared_ptr<client_context>;

struct value
{
    std::vector<std::byte> data;
    std::unordered_set<client_context_ptr> data_subscribers;
    std::unordered_set<client_context_ptr> stream_subscribers;
    int64_t vsn = 0;
    int64_t ssn = 0;
};

class value_map
{
public:
    value& get_value(uint16_t id)
    {
        if (id >= m_value_map.size())
        {
            m_value_map.reserve(id*2);
            m_value_map.resize(id+1);
        }

        return m_value_map[id];
    }

    bool set_value(uint16_t id, const bfc::const_buffer_view& data)
    {
        if (data.size() > 1024*8)
        {
            return false;
        }

        auto& value = get_value(id);
        value.data.resize(data.size());
        std::memcpy(value.data.data(), data.data(), data.size());
        value.vsn++;

        return true;
    }

private:
    std::vector<value> m_value_map;
};

} // namespace property

#endif // __VALUE_MAP_HPP__
