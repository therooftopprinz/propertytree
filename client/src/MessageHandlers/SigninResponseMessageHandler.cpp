#include "SigninResponseMessageHandler.hpp"

namespace ptree
{
namespace client
{

SigninResponseMessageHandler::
    SigninResponseMessageHandler(PTreeClient& pc, IEndPoint& ep):
        MessageHandler(pc, ep)
{}

inline void SigninResponseMessageHandler::handle(protocol::MessageHeaderPtr header, BufferPtr message)
{
    logger::Logger log("SigninResponseMessageHandler");

    log << logger::DEBUG << "signin response received.";

    protocol::SigninResponse response;
    protocol::Decoder de(message->data(),message->data()+message->size());
    response << de;
    {
        std::lock_guard<std::mutex> lock(ptreeClient.uuidMetaMapMutex);

        for (auto& i : *response.creations)
        {
            log << logger::DEBUG << " meta entry, path: " <<  *i.path << " id:"  << unsigned(*i.uuid) << " ptype: "<< unsigned(*i.propertyType);
            ptreeClient.uuidMetaMap[*i.uuid] = PTreeClient::PTreeMeta(*i.path, *i.propertyType);
            ptreeClient.pathUuidMap[*i.path] = *i.uuid;
        }

    };
    // std::map<protocol::Uuid, PTreeMeta> uuidMetaMap;
    // std::map<std::string, protocol::Uuid> pathUuidMap;
    
    // bool supported = true;
    // if (request.version != 1)
    // {
    //     log << logger::WARNING << "Version not supported: " <<
    //         request.version;
    //     supported = false;
    // }

    // clientServer.setUpdateInterval(request.refreshRate);
    // clientServer.clientSigned();

    // protocol::SigninResponse response;
    // response.version = supported ? *request.version : 0;
    // if (supported)
    // {
    //     auto meta = ptree.getPTreeInfo();
    //     for (const auto& i : meta)
    //     {
    //         response.creations->emplace_back(std::get<1>(i), std::get<2>(i), std::get<0>(i));
    //     }
    // }

    // messageSender(header->transactionId, protocol::MessageType::SigninResponse, response);
    // log << logger::DEBUG << "response size: " << response.size()+sizeof(protocol::MessageHeader);
    ptreeClient.notifyTransactionCV(header->transactionId);
}

} // namespace client
} // namespace ptree