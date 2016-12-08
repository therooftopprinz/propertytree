#ifndef SERVER_SERVERLET_MESSAGEHANDLERS_GETSPECIFICMETAREQUESTMESSAGEHANDLER_HPP_
#define SERVER_SERVERLET_MESSAGEHANDLERS_GETSPECIFICMETAREQUESTMESSAGEHANDLER_HPP_

#include "MessageHandler.hpp"

namespace ptree
{
namespace server
{

struct  GetSpecificMetaRequestMessageHandler : public MessageHandler
{
    GetSpecificMetaRequestMessageHandler(ClientServer& cs, IEndPoint& ep, core::PTree& pt, IClientServerMonitor& csmon);
    void handle(protocol::MessageHeaderPtr header, BufferPtr message);
};

} // namespace server
} // namespace ptree

#endif // SERVER_SERVERLET_MESSAGEHANDLERS_GETSPECIFICMETAREQUESTMESSAGEHANDLER_HPP_
