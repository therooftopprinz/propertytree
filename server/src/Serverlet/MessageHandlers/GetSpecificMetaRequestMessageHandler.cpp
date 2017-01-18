// #include "GetSpecificMetaRequestMessageHandler.hpp"

// #include <server/src/Serverlet/ClientServer.hpp>
// #include <common/src/Logger.hpp>
// #include <common/src/Utils.hpp>

// namespace ptree
// {
// namespace server
// {

// GetSpecificMetaRequestMessageHandler::GetSpecificMetaRequestMessageHandler
//     (ClientServer& cs, IEndPoint& ep, core::PTree& pt, IClientServerMonitor&  csmon):
//         MessageHandler(cs,ep,pt,csmon)
// {
// }

// void GetSpecificMetaRequestMessageHandler::handle(protocol::MessageHeaderPtr header, BufferPtr message)
// {
//     logger::Logger log("GetSpecificMetaRequestMessageHandler");

//     protocol::GetSpecificMetaRequest request;
//     request.unpackFrom(*message);

//     protocol::GetSpecificMetaResponse response;

//     log << logger::DEBUG << "Requesting for: " << request.path;

//     protocol::Uuid uuid = static_cast<protocol::Uuid>(-1);
//     protocol::PropertyType ptype = static_cast<protocol::PropertyType>(-1);

//     try
//     {
//         auto prop = ptree.getPropertyByPath<core::IProperty>(request.path);

//         if (prop)
//         {
//             uuid = prop->getUuid();
//             if (std::dynamic_pointer_cast<core::Value>(prop))
//             {
//                 ptype = protocol::PropertyType::Value;
//             }
//             else if(std::dynamic_pointer_cast<core::Rpc>(prop))
//             {
//                 ptype = protocol::PropertyType::Rpc;
//             }
//             else
//             {
//                 ptype = protocol::PropertyType::Node;
//             }
//         }

//     }
//     catch (core::ObjectNotFound)
//     {
//         log << logger::ERROR << "Object " << request.path << " not found.";
//     }
//     catch (core::MalformedPath)
//     {
//         log << logger::ERROR << "Malformed path " << request.path << " !";
//     }

//     response.meta = protocol::MetaCreate(uuid, ptype, request.path);

//     messageSender(header->transactionId, protocol::MessageType::GetSpecificMetaResponse, response);
// }

// } // namespace server
// } // namespace ptree