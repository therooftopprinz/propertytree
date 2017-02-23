#include "MessageHandlerFactory.hpp"
#include <common/src/Logger.hpp>
#include <client/src/TransactionsCV.hpp>
#include <client/src/IClientOutgoing.hpp>

namespace ptree
{
namespace client
{

struct MessageHandler;

std::unique_ptr<MessageHandler>
    MessageHandlerFactory::
        get(protocol::MessageType type, TransactionsCV& transactionsCV, LocalPTree& ptree, IClientOutgoing& outgoing)
{
    logger::Logger log("MessageHandlerFactory");
    switch (type)
    {
        case protocol::MessageType::SigninResponse:
        case protocol::MessageType::CreateResponse:
        case protocol::MessageType::DeleteResponse:
        case protocol::MessageType::GetSpecificMetaResponse:
        case protocol::MessageType::GetValueResponse:
        case protocol::MessageType::SubscribePropertyUpdateResponse:
        case protocol::MessageType::UnsubscribePropertyUpdateResponse:
        case protocol::MessageType::RpcResponse:
            log << logger::WARNING << "RpcResponse";
            return std::make_unique<GenericResponseMessageHandler>(transactionsCV);
        case protocol::MessageType::PropertyUpdateNotification:
            log << logger::WARNING << "PropertyUpdateNotification";
            return std::make_unique<PropertyUpdateNotificationMessageHandler>(transactionsCV, ptree);
        case protocol::MessageType::MetaUpdateNotification:
            log << logger::WARNING << "MetaUpdateNotification";
            return std::make_unique<MetaUpdateNotificationMessageHandler>(transactionsCV, ptree);
        case protocol::MessageType::HandleRpcRequest:
            log << logger::WARNING << "HandleRpcRequest";
            return std::make_unique<HandleRpcRequestMessageHandler>(transactionsCV, ptree, outgoing);
        default:
            break;
    }

    log << logger::ERROR << "Unregconize message type.";
    return std::unique_ptr<MessageHandler>();
}

} // namespace server
} // namespace ptree
