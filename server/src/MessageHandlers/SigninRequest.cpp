#include "SigninRequest.hpp"

namespace ptree
{
namespace server
{

SigninRequestHandler::SigninRequestHandler(IEndPointPtr endpoint, ClientServerPtr clientServer):
    endpoint(endpoint),
    clientServer(clientServer)
{

}

void SigninRequestHandler::handle(protocol::MessageHeaderPtr header, BufferPtr message)
{
    logger::Logger log("SigninRequest");

    protocol::SignInRequest &rqst = *(protocol::SignInRequest*)(message->data());
    log << logger::DEBUG << "SigninRequest";

    bool supported = true;
    if ( rqst.version != 1)
    {
        log << logger::DEBUG << "Version not supported: " <<
            rqst.version;
        supported = false;
    }

    clientServer->setUpdateInterval(rqst.updateInterval);
    clientServer->clientSigned();
    log << logger::DEBUG << "inteval: " << rqst.updateInterval;

    MessageSignInResponseSender rsp(header->transactionId, endpoint);
    rsp.setVersion(supported ? 1 : 0);
    rsp.send();
}

} // namespace server
} // namespace ptree