#include "MessageHandlerFactory.hpp"
#include <common/src/Logger.hpp>

namespace ptree
{
namespace client
{

class MessageHandler;

std::unique_ptr<MessageHandler>
    MessageHandlerFactory::
        get(protocol::MessageType type, PTreeClientPtr& cs, IEndPointPtr& ep)
{
    logger::Logger log("MessageHandlerFactory");
    using Enum = uint8_t;
    // switch (uint8_t(type))
    // {
    //     case (Enum) protocol::MessageType::SigninRequest:
    //         return std::make_unique<SigninRequestMessageHandler>(*cs.get(), *ep.get(), *pt.get(), *csmon.get());
    // }

    log << logger::ERROR << "Unregconize message type.";
    return std::make_unique<MessageHandler>(*cs.get(), *ep.get(), *pt.get(), *csmon.get());
}

} // namespace server
} // namespace ptree
