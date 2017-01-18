// #ifndef SERVER_SERVERLET_MESSAGEHANDLERS_SUBSCRIBEPROPERTYUPDATEREQUESTMESSAGEHANDLER_HPP_
// #define SERVER_SERVERLET_MESSAGEHANDLERS_SUBSCRIBEPROPERTYUPDATEREQUESTMESSAGEHANDLER_HPP_

// #include "MessageHandler.hpp"

// namespace ptree
// {
// namespace server
// {

// class  UpdateNotificationHandler : public std::enable_shared_from_this<UpdateNotificationHandler>
// {
// public:
//     UpdateNotificationHandler(ClientServerWkPtr clientServer);
//     bool handle(core::ValuePtr value);
// private:
//     ClientServerWkPtr clientServer;
// };

// struct SubscribePropertyUpdateRequestMessageHandler : public MessageHandler
// {
//     /** TODO: investigate why making ClientServerPtr not a ref causes a definitely lost. **/
//     SubscribePropertyUpdateRequestMessageHandler(ClientServerPtr& cs, IEndPoint& ep, core::PTree& pt, IClientServerMonitor& csmon);
//     void handle(protocol::MessageHeaderPtr header, BufferPtr message);
// private:
//     /** TODO: investigate why making ClientServerPtr not a ref causes a definitely lost. **/
//     ClientServerPtr& cs;
// };

// }
// }
// #endif // SERVER_SERVERLET_MESSAGEHANDLERS_SUBSCRIBEPROPERTYUPDATEREQUESTMESSAGEHANDLER_HPP_
