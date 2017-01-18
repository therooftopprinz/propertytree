#include "MessageHandlerFactory.hpp"
#include <common/src/Logger.hpp>

namespace ptree
{
namespace server
{

void MessageHandlerDummy::handle(protocol::MessageHeaderPtr, BufferPtr)
{}

std::unique_ptr<MessageHandler>
    MessageHandlerFactory::
        get(protocol::MessageType type, ClientServerConfig& config, IPTreeOutgoingPtr& outgoing, core::PTreePtr& ptree, IClientNotifierPtr& notifier)
{
    logger::Logger log("MessageHandlerFactory");
    using Enum = uint8_t;
    switch (uint8_t(type))
    {
        case (Enum) protocol::MessageType::SigninRequest:
            return std::make_unique<SigninRequestMessageHandler>(*outgoing, config, *ptree, *notifier);
        case (Enum) protocol::MessageType::CreateRequest:
            return std::make_unique<CreateRequestMessageHandler>(outgoing, *ptree, *notifier);
        case (Enum) protocol::MessageType::DeleteRequest:
            return std::make_unique<DeleteRequestMessageHandler>(*outgoing, *ptree, *notifier);
        case (Enum) protocol::MessageType::SetValueIndication:
            return std::make_unique<SetValueIndicationMessageHandler>(*ptree);
        // case (Enum) protocol::MessageType::SubscribePropertyUpdateRequest:
        //     return std::make_unique<SubscribePropertyUpdateRequestMessageHandler>(cs, *ep.get(), *pt.get(), *csmon.get());
        // case (Enum) protocol::MessageType::UnsubscribePropertyUpdateRequest:
        //     return std::make_unique<UnsubscribePropertyUpdateRequestMessageHandler>(*cs.get(), *ep.get(), *pt.get(), *csmon.get());
        // case (Enum) protocol::MessageType::GetValueRequest:
        //     return std::make_unique<GetValueRequestMessageHandler>(*cs.get(), *ep.get(), *pt.get(), *csmon.get());
        // case (Enum) protocol::MessageType::RpcRequest:
        //     return std::make_unique<RpcRequestMessageHandler>(cs, *ep.get(), *pt.get(), *csmon.get());
        // case (Enum) protocol::MessageType::HandleRpcResponse:
        //     return std::make_unique<HandleRpcResponseMessageHandler>(*cs.get(), *ep.get(), *pt.get(), *csmon.get());
        // case (Enum) protocol::MessageType::GetSpecificMetaRequest:
        //     return std::make_unique<GetSpecificMetaRequestMessageHandler>(*cs.get(), *ep.get(), *pt.get(), *csmon.get());
    }

    log << logger::ERROR << "Unregconize message type.";
    return std::make_unique<MessageHandlerDummy>();
}

} // namespace server
} // namespace ptree
