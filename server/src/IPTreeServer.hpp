#ifndef SERVER_IPTREESERVER_HPP_
#define SERVER_IPTREESERVER_HPP_

#include <string>
#include <server/src/Types.hpp>
#include <interface/protocol.hpp>

namespace ptree
{
namespace server
{

class IPTreeServer
{
public:
    IPTreeServer() = default;
    virtual ~IPTreeServer() = default;
    virtual void addClientServer(uint64_t clienServerId, ClientServerPtr& clientNotifier) = 0;
    virtual void removeClientServer(uint64_t clienServerId) = 0;
    virtual void notifyCreation(uint32_t uuid, protocol::PropertyType type, const std::string& path) = 0;
    virtual void notifyDeletion(uint32_t uuid) = 0;
    virtual void notifyRpcResponse(uint64_t csId, uint32_t transactionId, Buffer&& returnValue) = 0;
};

} // namespace server
} // namespace ptree

#endif  // SERVER_IPTREESERVER_HPP_