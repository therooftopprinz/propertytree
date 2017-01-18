#include "SigninRequestMessageHandler.hpp"
#include <server/src/Serverlet/ClientServer.hpp>
#include <common/src/Logger.hpp>

namespace ptree
{
namespace server
{

SigninRequestMessageHandler::SigninRequestMessageHandler(IPTreeOutgoing& outgoing, ClientServerConfig& config,
     core::PTree& ptree, IClientNotifier& notifier):
        outgoing(outgoing), config(config), ptree(ptree), notifier(notifier)
{}

inline void SigninRequestMessageHandler::handle(protocol::MessageHeaderPtr header, BufferPtr message)
{
    logger::Logger log("SigninRequestMessageHandler");

    protocol::SigninRequest request;
    request.unpackFrom(*message);

    bool supported = true;
    if (request.version != 1)
    {
        log << logger::WARNING << "Version not supported: " <<
            request.version;
        supported = false;
    }

    config.updateInterval = request.refreshRate;
    config.enableOutgoing = true;

    protocol::SigninResponse response;
    response.version = supported ? request.version : 0;
    // if (supported)
    // {
    //     auto meta = ptree.getPTreeInfo();
    //     for (const auto& i : meta)
    //     {
    //         response.creations->emplace_back(std::get<1>(i), std::get<2>(i), std::get<0>(i));
    //     }
    // }

    outgoing.sendToClient(header->transactionId, protocol::MessageType::SigninResponse, response);
    log << logger::DEBUG << "response size: " << response.size()+sizeof(protocol::MessageHeader);
}

} // namespace server
} // namespace ptree