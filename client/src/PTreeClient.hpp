#ifndef CLIENT_PTREECLIENT_HPP_
#define CLIENT_PTREECLIENT_HPP_

#include <cassert>
#include <map>
#include <vector>
#include <string>
#include <mutex>
#include <thread>
#include <memory>
#include <atomic>
#include <algorithm>
#include <condition_variable>
#include <interface/protocol.hpp>
#include <common/src/Logger.hpp>
#include <common/src/IEndPoint.hpp>
#include <client/src/TransactionsCV.hpp>
#include <client/src/ClientOutgoing.hpp>
#include <client/src/ClientIncoming.hpp>
#include <client/src/LocalPTree.hpp>

namespace ptree
{
namespace client
{

class ValueContainer;
typedef std::shared_ptr<ValueContainer> ValueContainerPtr;
class RpcContainer;
typedef std::shared_ptr<RpcContainer> RpcContainerPtr;


struct IMetaUpdateHandler;
class PTreeClient : public std::enable_shared_from_this<PTreeClient>
{
public:
    PTreeClient(common::IEndPointPtr endpoint);
    ~PTreeClient();
    LocalPTreePtr getPTree();

private:

    common::IEndPointPtr endpoint;
    TransactionsCV transactionsCV;
    ClientOutgoing outgoing;
    LocalPTree ptree;
    ClientIncoming incoming;
    logger::Logger log;
};

}
}

#endif  // CLIENT_PTREECLIENT_HPP_
