#ifndef SERVER_SERVERLET_MESSAGEHANDLERS_SETVALUEINDICATIONMESSAGEHANDLER_HPP_
#define SERVER_SERVERLET_MESSAGEHANDLERS_SETVALUEINDICATIONMESSAGEHANDLER_HPP_

#include "MessageHandler.hpp"

namespace ptree
{
namespace server
{

class SetValueIndicationMessageHandler : public MessageHandler
{
public:
    SetValueIndicationMessageHandler(core::PTree& ptree);
    void handle(protocol::MessageHeader& header, Buffer& message);
private:
    core::PTree& ptree;
};

}
}
#endif // SERVER_SERVERLET_MESSAGEHANDLERS_SETVALUEINDICATIONMESSAGEHANDLER_HPP_
