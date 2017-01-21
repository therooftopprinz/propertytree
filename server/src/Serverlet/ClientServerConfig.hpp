#ifndef SERVER_CLIENTSERVERCONFIG_HPP_
#define SERVER_CLIENTSERVERCONFIG_HPP_

#include <cstdint>

namespace ptree
{
namespace server
{


struct ClientServerConfig
{
    bool enableOutgoing = false;
    uint32_t updateInterval = 200*1000;
};

} // namespace server
} // namespace ptree

#endif  // SERVER_CLIENTSERVERCONFIG_HPP_