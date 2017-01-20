#include <sstream>
#include <iostream>
#include <algorithm>
#include <thread>
#include "ClientServer.hpp"
#include <common/src/Utils.hpp>

namespace ptree
{
namespace server
{

ClientNotifier::ClientNotifier() :
    log(logger::Logger("ClientNotifier"))
{
    log << logger::DEBUG << "construct";
}

ClientNotifier::~ClientNotifier()
{
    log << logger::DEBUG << "destruct";
}

void ClientNotifier::addClientServer(uint64_t clientServerId, IPTreeOutgoing& clientNotifier)
{
    std::lock_guard<std::mutex> guard(clientNotifierMutex);
    log << logger::DEBUG << "adding client server to notifier, csid= " << clientServerId;
    clientNotifiers.emplace(clientServerId, clientNotifier);
}

void ClientNotifier::removeClientServer(uint64_t clientServerId)
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

void ClientNotifier::notifyCreation(uint32_t uuid, protocol::PropertyType type, std::string path)
{
    std::lock_guard<std::mutex> guard(clientNotifierMutex);
    for (auto& clientNotifier : clientNotifiers)
    {
        clientNotifier.second.get().notifyCreation(uuid, type, path);
    }
}

void ClientNotifier::notifyDeletion(uint32_t uuid)
{
    std::lock_guard<std::mutex> guard(clientNotifierMutex);
    for (auto& clientNotifier : clientNotifiers)
    {
        clientNotifier.second.get().notifyDeletion(uuid);
    }
}

void ClientNotifier::notifyRpcResponse(uint64_t clientServerId, uint32_t transactionId, Buffer&& returnValue)
{
    std::lock_guard<std::mutex> guard(clientNotifierMutex);
    auto it = clientNotifiers.find(clientServerId);
    if (it != clientNotifiers.end())
    {
        it->second.get().notifyRpcResponse(transactionId, std::move(returnValue));;
    }
}


ClientServer::ClientServer(IEndPointPtr endpoint, core::PTreePtr ptree, IClientNotifierPtr notifier):
    clientServerId(std::uintptr_t(this)),
    endpoint(endpoint),
    outgoing(config, this->endpoint),
    ptree(ptree),
    notifier(notifier),
    incoming(clientServerId, config, this->endpoint, this->ptree, this->notifier),
    log(logger::Logger("ClientServer"))
{
    log << logger::DEBUG << "setup: adding to notifier";
    notifier->addClientServer(clientServerId, outgoing);
}

void ClientServer::setup()
{
    log << logger::DEBUG << "construct";
    log << logger::DEBUG << "initializing incoming...";
    incoming.setup(std::shared_ptr<IPTreeOutgoing>(shared_from_this(), &outgoing));
    outgoing.setup();
}

void ClientServer::teardown()
{
    log << logger::DEBUG << "destruct";
    incoming.teardown();
    outgoing.teardown();
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(1s); // wait setup to finish
    log << logger::DEBUG << "teardown: removing from notifier";
    notifier->removeClientServer(clientServerId);
}

ClientServer::~ClientServer()
{

}

} // namespace server
} // namespace ptree
