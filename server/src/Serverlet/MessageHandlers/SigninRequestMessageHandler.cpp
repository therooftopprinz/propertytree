#include "SigninRequestMessageHandler.hpp"
#include <server/src/Serverlet/ClientServer.hpp>

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
    logger::Logger log("SigninRequest");

    protocol::SigninRequest request;
    protocol::Decoder de(message->data(),message->data()+message->size());
    request << de;

    bool supported = true;
    if (request.version != 1)
    {
        log << logger::DEBUG << "Version not supported: " <<
            request.version;
        supported = false;
    }

    clientServer.setUpdateInterval(request.refreshRate);
    clientServer.clientSigned();
    log << logger::DEBUG << "inteval: " << request.refreshRate;

    protocol::SigninResponse response;
    response.version = supported ? *request.version : 0;

    Buffer responseMessageBuffer(response.size());
    protocol::BufferView responseMessageBufferView(responseMessageBuffer);
    protocol::Encoder en(responseMessageBufferView);
    response >> en;
    endpoint.send(responseMessageBuffer.data(), responseMessageBuffer.size());
}

} // namespace server
} // namespace ptree