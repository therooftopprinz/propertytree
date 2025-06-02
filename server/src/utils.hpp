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

} // propertytree

#endif // __UTILS_HPP__
