#ifndef CLIENT_RPCCONTAINER_HPP_
#define CLIENT_RPCCONTAINER_HPP_

#include <cassert>
#include <map>
#include <vector>
#include <set>
#include <string>
#include <cstring>
#include <mutex>
#include <thread>
#include <memory>
#include <functional>

#include <interface/protocol.hpp>
#include <common/src/Logger.hpp>
#include <client/src/IProperty.hpp>
#include <client/src/Types.hpp>
#include <client/src/LocalPTree.hpp>

namespace ptree
{
namespace client
{
class LocalPTree;
class RpcContainer : public IProperty, public std::enable_shared_from_this<RpcContainer>
{
public:
    RpcContainer() = delete;

    void setHandler(std::function<Buffer(Buffer&)>);
    void setVoidHandler(std::function<void(Buffer&)>);

    template <typename T>
    Buffer call(T&& parameter)
    {
        Buffer tmv(sizeof(T));
        std::memcpy(tmv.data(), &parameter, sizeof(T));
        return call(std::move(tmv));
    }

    RpcContainer(LocalPTree& ptree, protocol::Uuid uuid, std::string path, bool owned);
    RpcContainer(LocalPTree& ptree, protocol::Uuid uuid, std::string path, std::function<Buffer(Buffer&)> handler, std::function<void(Buffer&)> voidHandler, bool owned);

private:
    Buffer call(Buffer&& parameter);
    LocalPTree& ptree;
    std::function<Buffer(Buffer&)> handler;
    std::function<void(Buffer&)> voidHandler;
    logger::Logger log;

    friend LocalPTree;
};

}
}

#endif // CLIENT_RPCCONTAINER_HPP_
