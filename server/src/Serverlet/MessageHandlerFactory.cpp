#include "MessageHandlerFactory.hpp"

namespace ptree
{
namespace server
{

class MessageHandler;

MessageHandler
    MessageHandlerFactory::
        get(protocol::MessageType type, ClientServer& cs, IEndPoint& ep, core::PTree& pt, IClientServerMonitor&  csmon)
{
    using Enum = uint8_t;
    switch (uint8_t(type))
    {
        case (Enum) protocol::MessageType::SigninRequest:
            return SigninRequestMessageHandler(cs, ep, pt, csmon);
    }
    // if (type == protocol::MessageType::SignInRequest)
    // {
    //     std::lock_guard<std::mutex> guard(sendLock);
    //     SigninRequestHandler mh(endpoint, shared_from_this());
    //     mh.handle(header, message);
    // }
    // else if (type == protocol::MessageType::CreateRequest)
    // {
    //     std::lock_guard<std::mutex> guard(sendLock);
    //     CreateRequestHandler mh(endpoint, ptree, monitor, shared_from_this());
    //     mh.handle(header, message);
    // }
    // else if (type == protocol::MessageType::DeleteRequest)
    // {
    //     std::lock_guard<std::mutex> guard(sendLock);
    //     DeleteRequestHandler mh(endpoint, ptree, monitor, shared_from_this());
    //     mh.handle(header, message);
    // }
    // else if (type == protocol::MessageType::SetValueIndication)
    // {
    //     SetValueIndicationHandler mh(endpoint, ptree, monitor, shared_from_this());
    //     mh.handle(header, message);
    // }
    // else if (type == protocol::MessageType::SubscribePropertyUpdateRequest)
    // {
    //     std::lock_guard<std::mutex> guard(sendLock);
    //     SubscribePropertyUpdateRequestHandler mh(endpoint, ptree, shared_from_this());
    //     mh.handle(header, message);
    // }
    // else if (type == protocol::MessageType::UnsubscribePropertyUpdateRequest)
    // {
    //     std::lock_guard<std::mutex> guard(sendLock);
    //     UnsubscribePropertyUpdateRequestHandler mh(endpoint, ptree, shared_from_this());
    //     mh.handle(header, message);
    // }
    // else if (type == protocol::MessageType::GetValueRequest)
    // {
    //     std::lock_guard<std::mutex> guard(sendLock);
    //     GetValueRequestHandler mh(endpoint, ptree, shared_from_this());
    //     mh.handle(header, message);
    // }
    // else
    // {
    //     log << logger::ERROR << "processMessage: header invalid type!! ";
    // }
    return MessageHandler(cs, ep, pt, csmon);
}

} // namespace server
} // namespace ptree
