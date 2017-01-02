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

struct IValueWatcher
{
    virtual void handle(ValueContainerPtr vc) = 0;
};

class RpcContainer : public std::enable_shared_from_this<RpcContainer>
{
public:
    RpcContainer() = delete;

    /** TODO: on destruction if meta uuid is not watched delete meta. **/

    RpcContainer(protocol::Uuid uuid, bool ownership);
    Buffer operator()(Buffer args);

private:
    std::function<Buffer(Buffer&)> handler;
    protocol::Uuid uuid;
    bool ownership;
    logger::Logger log;

    friend class PTreeClient;
};

}
}

#endif // CLIENT_RPCCONTAINER_HPP_
