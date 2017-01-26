#include "MessageHandlerFactory.hpp"
#include <common/src/Logger.hpp>
#include <client/src/TransactionsCV.hpp>

namespace ptree
{
namespace client
{

struct MessageHandler;

std::unique_ptr<MessageHandler>
    MessageHandlerFactory::
        get(protocol::MessageType type, TransactionsCV& transactionsCV, IEndPoint& endpoint)
{
    logger::Logger log("MessageHandlerFactory");
    using Enum = uint8_t;
    switch (uint8_t(type))
    {
        case (Enum) protocol::MessageType::SigninResponse:
        case (Enum) protocol::MessageType::CreateResponse:
        case (Enum) protocol::MessageType::GetSpecificMetaResponse:
        case (Enum) protocol::MessageType::GetValueResponse:
        case (Enum) protocol::MessageType::SubscribePropertyUpdateResponse:
        case (Enum) protocol::MessageType::UnsubscribePropertyUpdateResponse:
        case (Enum) protocol::MessageType::RpcResponse:
            return std::make_unique<GenericResponseMessageHandler>(transactionsCV);
    //     case (Enum) protocol::MessageType::PropertyUpdateNotification:
    //         return std::make_unique<PropertyUpdateNotificationMessageHandler>(*pc.get(), *ep.get());
    //     case (Enum) protocol::MessageType::MetaUpdateNotification:
    //         return std::make_unique<MetaUpdateNotificationMessageHandler>(*pc.get(), *ep.get());
    //     case (Enum) protocol::MessageType::HandleRpcRequest:
    //         return std::make_unique<HandleRpcRequestMessageHandler>(*pc.get(), *ep.get());
    }

    log << logger::ERROR << "Unregconize message type.";
    return std::unique_ptr<MessageHandler>();
}

} // namespace server
} // namespace ptree
