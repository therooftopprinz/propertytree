#include "MessageHandlerFactory.hpp"
#include <server/src/Logger.hpp>

namespace ptree
{
namespace server
{

class MessageHandler;

std::unique_ptr<MessageHandler>
    MessageHandlerFactory::
        get(protocol::MessageType type, ClientServerPtr& cs, IEndPointPtr& ep, core::PTreePtr& pt, IClientServerMonitorPtr&  csmon)
{
    logger::Logger log("MessageHandlerFactory");
    using Enum = uint8_t;
    switch (uint8_t(type))
    {
        case (Enum) protocol::MessageType::SigninRequest:
            return std::make_unique<SigninRequestMessageHandler>(*cs.get(), *ep.get(), *pt.get(), *csmon.get());
        case (Enum) protocol::MessageType::CreateRequest:
            return std::make_unique<CreateRequestMessageHandler>(*cs.get(), *ep.get(), *pt.get(), *csmon.get());
        case (Enum) protocol::MessageType::DeleteRequest:
            return std::make_unique<DeleteRequestMessageHandler>(*cs.get(), *ep.get(), *pt.get(), *csmon.get());
        case (Enum) protocol::MessageType::SetValueIndication:
            return std::make_unique<SetValueIndicationMessageHandler>(*cs.get(), *ep.get(), *pt.get(), *csmon.get());
        case (Enum) protocol::MessageType::SubscribePropertyUpdateRequest:
            return std::make_unique<SubscribePropertyUpdateRequestMessageHandler>(cs, *ep.get(), *pt.get(), *csmon.get());
        case (Enum) protocol::MessageType::UnsubscribePropertyUpdateRequest:
            return std::make_unique<UnsubscribePropertyUpdateRequestMessageHandler>(*cs.get(), *ep.get(), *pt.get(), *csmon.get());
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
    log << logger::ERROR << "Unregconize message type.";
    return std::make_unique<MessageHandler>(*cs.get(), *ep.get(), *pt.get(), *csmon.get());
}

} // namespace server
} // namespace ptree
