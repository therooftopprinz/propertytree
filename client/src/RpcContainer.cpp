#include "RpcContainer.hpp"

namespace ptree
{
namespace client
{

void RpcContainer::setHandler(std::function<Buffer(Buffer&)>)
{

}
void RpcContainer::setVoidHandler(std::function<void(Buffer&)>)
{

}

RpcContainer::RpcContainer(LocalPTree& ptree, protocol::Uuid uuid, const std::string& path, bool owned):
    IProperty(uuid, path, protocol::PropertyType::Rpc, owned),
    ptree(ptree), log("RpcContainer")
{
}
RpcContainer::RpcContainer(LocalPTree& ptree, protocol::Uuid uuid, const std::string& path, std::function<Buffer(Buffer&)> handler, std::function<void(Buffer&)> voidHandler, bool owned):
    IProperty(uuid, path, protocol::PropertyType::Rpc, owned),
    ptree(ptree), handler(handler), voidHandler(voidHandler), log("RpcContainer")
{
}

Buffer RpcContainer::call(Buffer&& parameter)
{
    return ptree.rpcRequest(getUuid(), std::move(parameter));
}


} // namesoace client
} // namesoace ptree