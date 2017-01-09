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
protocol::Uuid RpcContainer::getUuid()
{
    return uuid;
}
RpcContainer::RpcContainer(protocol::Uuid uuid, bool owned):
    uuid(uuid), owned(owned), log("RpcContainer")
{
}
RpcContainer::RpcContainer(protocol::Uuid uuid, std::function<Buffer(Buffer&)> handler, std::function<void(Buffer&)> voidHandler, bool owned):
    handler(handler), voidHandler(voidHandler), uuid(uuid), owned(owned), log("RpcContainer")
{
}

} // namesoace client
} // namesoace ptree