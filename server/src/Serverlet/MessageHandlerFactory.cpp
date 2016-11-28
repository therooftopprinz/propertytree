#include "MessageHandlerFactory.hpp"
#include <common/src/Logger.hpp>

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
            return std::make_unique<CreateRequestMessageHandler>(cs, *ep.get(), *pt.get(), *csmon.get());
        case (Enum) protocol::MessageType::DeleteRequest:
            return std::make_unique<DeleteRequestMessageHandler>(*cs.get(), *ep.get(), *pt.get(), *csmon.get());
        case (Enum) protocol::MessageType::SetValueIndication:
            return std::make_unique<SetValueIndicationMessageHandler>(*cs.get(), *ep.get(), *pt.get(), *csmon.get());
        case (Enum) protocol::MessageType::SubscribePropertyUpdateRequest:
            return std::make_unique<SubscribePropertyUpdateRequestMessageHandler>(cs, *ep.get(), *pt.get(), *csmon.get());
        case (Enum) protocol::MessageType::UnsubscribePropertyUpdateRequest:
            return std::make_unique<UnsubscribePropertyUpdateRequestMessageHandler>(*cs.get(), *ep.get(), *pt.get(), *csmon.get());
        case (Enum) protocol::MessageType::GetValueRequest:
            return std::make_unique<GetValueRequestMessageHandler>(*cs.get(), *ep.get(), *pt.get(), *csmon.get());
        case (Enum) protocol::MessageType::RpcRequest:
            return std::make_unique<RpcRequestMessageHandler>(cs, *ep.get(), *pt.get(), *csmon.get());
        case (Enum) protocol::MessageType::HandleRpcResponse:
            return std::make_unique<HandleRpcResponseMessageHandler>(*cs.get(), *ep.get(), *pt.get(), *csmon.get());
    }

    log << logger::ERROR << "Unregconize message type.";
    return std::make_unique<MessageHandler>(*cs.get(), *ep.get(), *pt.get(), *csmon.get());
}

} // namespace server
} // namespace ptree
