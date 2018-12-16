#ifndef SERVER_PTREESERVER_HPP_
#define SERVER_PTREESERVER_HPP_

#include <sys/types.h>
#include <sys/socket.h>
#include <common/src/IEndPoint.hpp>
#include <common/src/TcpEndPoint.hpp>
#include <server/src/Serverlet/IPTreeOutgoing.hpp>
#include <server/src/PTree.hpp>
#include "Types.hpp"
#include <common/src/Logger.hpp>
#include <server/src/IPTreeServer.hpp>

namespace ptree
{
namespace server
{

class PTreeServer : public IPTreeServer
{
public:
    PTreeServer();
    ~PTreeServer();
    void addClientServer(uint64_t clientServerId, ClientServerPtr& clientNotifier);
    void removeClientServer(uint64_t clientServerId);
    void notifyCreation(uint32_t uuid, protocol::PropertyType type, const std::string& path);
    void notifyDeletion(uint32_t uuid);
    void notifyRpcResponse(uint64_t clientServerId, uint32_t transactionId, Buffer&& returnValue);
private:
    std::map<uint64_t, ClientServerPtr> clientNotifiers;
    std::mutex clientNotifierMutex;
    logger::Logger log;
};

} // namespace server
} // namespace ptree

#endif  // SERVER_PTREESERVER_HPP_