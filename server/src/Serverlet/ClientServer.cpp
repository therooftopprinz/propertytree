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

ClientServer::ClientServer(IEndPointPtr endpoint, core::PTree& ptree, IPTreeServer& notifier):
    endpoint(endpoint),
    outgoing(std::make_shared<PTreeOutgoing>(config, *endpoint)),
    incoming(uintptr_t(this), config, *endpoint, outgoing, ptree, notifier),
    log("ClientServer")
{}

ClientServer::~ClientServer()
{
    log << logger::DEBUG << "destruct";
    using namespace std::chrono_literals;
    // TODO: REMOVE THIS SLEEP!!!
    std::this_thread::sleep_for(1s); // wait setup to finish
    // log << logger::DEBUG << "teardown: removing from notifier";
}

} // namespace server
} // namespace ptree
