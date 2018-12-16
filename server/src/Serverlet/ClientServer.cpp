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

ClientServer::ClientServer(IEndPointPtr endpoint, core::PTreePtr ptree, IPTreeServer& notifier):
    clientServerId(std::uintptr_t(this)),
    endpoint(endpoint),
    outgoing(config, this->endpoint),
    ptree(ptree),
    notifier(notifier),
    incoming(clientServerId, config, this->endpoint, outgoing, this->ptree, this->notifier),
    log(logger::Logger("ClientServer"))
{}

void ClientServer::init()
{
    log << logger::DEBUG << "initializing incoming...";
    auto spt = shared_from_this();
    incoming.init(std::shared_ptr<IPTreeOutgoing>(spt, &outgoing));
    log << logger::DEBUG << "setup: adding to notifier";
    notifier.addClientServer(clientServerId, spt);
}

ClientServer::~ClientServer()
{
    log << logger::DEBUG << "destruct";
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(1s); // wait setup to finish
    log << logger::DEBUG << "teardown: removing from notifier";
}

} // namespace server
} // namespace ptree
