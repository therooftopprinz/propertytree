#ifndef CLIENT_LOCALPTREE_HPP_
#define CLIENT_LOCALPTREE_HPP_

#include <interface/protocol.hpp>
#include <string>
#include <client/src/ValueContainer.hpp>
#include <client/src/RpcContainer.hpp>

namespace ptree
{
namespace client
{


class ValueContainer;
typedef std::shared_ptr<ValueContainer> ValueContainerPtr;
class RpcContainer;
typedef std::shared_ptr<RpcContainer> RpcContainerPtr;

class LocalPTree
{
public:
    ValueContainerPtr createValue(std::string path, Buffer value);
    RpcContainerPtr createRpc(std::string path, std::function<Buffer(Buffer&)> handler, std::function<void(Buffer&)> voidHandler);
    bool createNode(std::string path);
    ValueContainerPtr getValue(std::string& path);
    RpcContainerPtr getRpc(std::string& path);
private:
    void addMeta(protocol::Uuid uuid, std::string path, protocol::PropertyType type);
    void removeMeta(protocol::Uuid uuid);
};
}
}
#endif  // CLIENT_LOCALPTREE_HPP_
