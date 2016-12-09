#include "SigninRequestMessageHandler.hpp"
#include <server/src/Serverlet/ClientServer.hpp>
#include <common/src/Logger.hpp>

namespace ptree
{
namespace server
{

SigninRequestMessageHandler::
    SigninRequestMessageHandler(ClientServer& cs, IEndPoint& ep, core::PTree& pt, IClientServerMonitor&  csmon):
        MessageHandler(cs,ep,pt,csmon)
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

    clientServer.setUpdateInterval(request.refreshRate);
    clientServer.clientSigned();

    protocol::SigninResponse response;
    response.version = supported ? *request.version : 0;
    // if (supported)
    // {
    //     auto meta = ptree.getPTreeInfo();
    //     for (const auto& i : meta)
    //     {
    //         response.creations->emplace_back(std::get<1>(i), std::get<2>(i), std::get<0>(i));
    //     }
    // }

    messageSender(header->transactionId, protocol::MessageType::SigninResponse, response);
    log << logger::DEBUG << "response size: " << response.size()+sizeof(protocol::MessageHeader);
}

} // namespace server
} // namespace ptree