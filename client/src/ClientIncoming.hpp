#ifndef CLIENT_CLIENTINCOMING_HPP_
#define CLIENT_CLIENTINCOMING_HPP_

#include <common/src/Logger.hpp>
#include "IClientOutgoing.hpp"
#include <common/src/IEndPoint.hpp>
#include <client/src/TransactionsCV.hpp>
#include <client/src/Types.hpp>
#include "MessageHandlerFactory.hpp"

namespace ptree
{
namespace client
{

class ClientIncoming
{
public:
    ClientIncoming(TransactionsCV& transactionsCV, common::IEndPoint& enpoint, LocalPTree& ptree);
    ~ClientIncoming();
private:
    void processMessage(protocol::MessageHeader& header, Buffer& message);
    void handleIncoming();
    bool killHandleIncoming;
    bool handleIncomingIsRunning;
    TransactionsCV& transactionsCV;
    common::IEndPoint& endpoint;
    LocalPTree& ptree;
    logger::Logger log;
};

}
}

#endif  // CLIENT_CLIENTINCOMING_HPP_
