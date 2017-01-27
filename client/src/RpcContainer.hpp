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

namespace ptree
{
namespace client
{

class RpcContainer : public IProperty, public std::enable_shared_from_this<RpcContainer>
{
public:
    RpcContainer() = delete;

    void setHandler(std::function<Buffer(Buffer&)>);
    void setVoidHandler(std::function<void(Buffer&)>);

    RpcContainer(protocol::Uuid uuid, std::string path, bool owned);
    RpcContainer(protocol::Uuid uuid, std::string path, std::function<Buffer(Buffer&)> handler, std::function<void(Buffer&)> voidHandler, bool owned);

private:
    std::function<Buffer(Buffer&)> handler;
    std::function<void(Buffer&)> voidHandler;
    logger::Logger log;
};

}
}

#endif // CLIENT_RPCCONTAINER_HPP_
