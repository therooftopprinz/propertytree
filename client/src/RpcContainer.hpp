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
#include <client/src/Types.hpp>

namespace ptree
{
namespace client
{

class RpcContainer : public std::enable_shared_from_this<RpcContainer>
{
public:
    RpcContainer() = delete;

    /** TODO: on destruction if meta uuid is not watched delete meta. **/
    void setHandler(std::function<Buffer(Buffer&)>);
    void setVoidHandler(std::function<void(Buffer&)>);
    protocol::Uuid getUuid();

    RpcContainer(protocol::Uuid uuid, bool owned);
    RpcContainer(protocol::Uuid uuid, std::function<Buffer(Buffer&)> handler, std::function<void(Buffer&)> voidHandler, bool owned);
private:
    std::function<Buffer(Buffer&)> handler;
    std::function<void(Buffer&)> voidHandler;
    protocol::Uuid uuid;
    bool owned;
    logger::Logger log;

    friend class PTreeClient;
};

}
}

#endif // CLIENT_RPCCONTAINER_HPP_
