#include <server/src/PTreeServer.hpp>
#include <server/src/Serverlet/ClientServer.hpp>

namespace ptree
{
namespace server
{

PTreeServer::PTreeServer() :
    log(logger::Logger("PTreeServer"))
{
    log << logger::DEBUG << "construct";
}

PTreeServer::~PTreeServer()
{
    log << logger::DEBUG << "destruct";
}

std::shared_ptr<ClientServer> PTreeServer::create(IEndPointPtr endpoint, core::PTree& ptree)
{
    auto cs = std::make_shared<ClientServer>(endpoint, ptree, *this);
    log << logger::DEBUG << "create...";
    addClientServer(std::uintptr_t(cs.get()), cs);
    return cs;
}

void PTreeServer::addClientServer(uint64_t clientServerId, ClientServerPtr& clientNotifier)
{
    std::lock_guard<std::mutex> guard(clientNotifierMutex);
    log << logger::DEBUG << "adding client server to notifier, csid= " << clientServerId;
    clientNotifiers.emplace(clientServerId, clientNotifier);
}

void PTreeServer::removeClientServer(uint64_t clientServerId)
{
    std::lock_guard<std::mutex> guard(clientNotifierMutex);
    log << logger::DEBUG << "removing client server to notifier, csid= " << clientServerId;
    auto it = clientNotifiers.find(clientServerId);
    if (it != clientNotifiers.end())
    {
        clientNotifiers.erase(it);
        log << logger::DEBUG << "erased! csid=" << clientServerId;
    }
    else
    {
        log << logger::ERROR << "not found! csid=" << clientServerId;
    }
}

void PTreeServer::notifyCreation(uint32_t uuid, protocol::PropertyType type, const std::string& path)
{
    std::lock_guard<std::mutex> guard(clientNotifierMutex);
    for (auto& clientNotifier : clientNotifiers)
    {
        clientNotifier.second.get()->getOutgoing().notifyCreation(uuid, type, path);
    }
}

void PTreeServer::notifyDeletion(uint32_t uuid)
{
    std::lock_guard<std::mutex> guard(clientNotifierMutex);
    for (auto& clientNotifier : clientNotifiers)
    {
        clientNotifier.second.get()->getOutgoing().notifyDeletion(uuid);
    }
}

void PTreeServer::notifyRpcResponse(uint64_t clientServerId, uint32_t transactionId, Buffer&& returnValue)
{
    std::lock_guard<std::mutex> guard(clientNotifierMutex);
    auto it = clientNotifiers.find(clientServerId);
    if (it != clientNotifiers.end())
    {
        it->second.get()->getOutgoing().notifyRpcResponse(transactionId, std::move(returnValue));;
    }
}


} // namespace server
} // namespace ptree
