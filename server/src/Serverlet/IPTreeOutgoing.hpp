#ifndef SERVER_IPTREEOUTGOING_HPP_
#define SERVER_IPTREEOUTGOING_HPP_

#include <string>
#include <interface/protocol.hpp>
#include <server/src/Types.hpp>

namespace ptree
{
namespace server
{   

struct IPTreeOutgoing
{
	virtual ~IPTreeOutgoing() {}

    virtual void notifyCreation(uint32_t uuid, protocol::PropertyType type, std::string path) = 0;
    virtual void notifyDeletion(uint32_t uuid) = 0;
    virtual void notifyValueUpdate(core::ValuePtr) = 0;
    virtual void notifyRpcRequest(protocol::Uuid uuid, uint64_t clientServerId, uint32_t transactionId, Buffer&& parameter) = 0;
    virtual void notifyRpcResponse(uint32_t transactionId, Buffer&& returnValue) = 0;
};

} // namespace server
} // namespace ptree

#endif  // SERVER_IPTREEOUTGOING_HPP_