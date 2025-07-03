#ifndef __UTILS_HPP__
#define __UTILS_HPP__

#include <bfc/configuration_parser.hpp>
#include <optional>

namespace utils
{

template<typename T>
std::optional<T> get_config(const bfc::configuration_parser& config, const std::string_view& name)
{
    auto def = config.as<T>(name);
    auto alt = config.as<T>("--" + std::string(name));
    if (alt)
    {
        return alt;
    }
    return def;
}
inline std::pair<uint32_t, uint16_t> get_addr(sockaddr* addr)
{
    if (addr->sa_family = AF_INET)
    {
        auto addr_in = (sockaddr_in*) addr;
        return std::make_pair(addr_in->sin_addr.s_addr, addr_in->sin_port);
    }
    return {};
}

} // propertytree

#endif // __UTILS_HPP__
