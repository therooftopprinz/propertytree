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

RpcContainer::RpcContainer(protocol::Uuid uuid, std::string path, bool owned):
    IProperty(uuid, path, protocol::PropertyType::Rpc, owned), log("RpcContainer")
{
}
RpcContainer::RpcContainer(protocol::Uuid uuid, std::string path, std::function<Buffer(Buffer&)> handler, std::function<void(Buffer&)> voidHandler, bool owned):
    IProperty(uuid, path, protocol::PropertyType::Rpc, owned), handler(handler), voidHandler(voidHandler), log("RpcContainer")
{
}

} // namesoace client
} // namesoace ptree