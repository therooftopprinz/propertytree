#ifndef CLIENT_ICLIENTOUTGOING_HPP_
#define CLIENT_ICLIENTOUTGOING_HPP_

#include <interface/protocol.hpp>
#include <common/src/IEndPoint.hpp>
#include <client/src/TransactionsCV.hpp>
#include <cstdint>
#include <list>

namespace ptree
{
namespace client
{
struct IClientOutgoing

{
    virtual std::pair<uint32_t,std::shared_ptr<TransactionCV>>
        signinRequest(int refreshRate, const std::list<protocol::SigninRequest::FeatureFlag> features) = 0;
    // virtual uint32_t createRequest() = 0;
    // virtual uint32_t deleteRequest() = 0;
    // virtual uint32_t setValueIndicationRequest() = 0;
    // virtual uint32_t subscribePropertyUpdateRequest() = 0;
    // virtual uint32_t unsubscribePropertyUpdateRequest() = 0;
    // virtual uint32_t getValueRequest() = 0;
    // virtual uint32_t rpcRequest() = 0;
    // virtual uint32_t handleRpcRequest() = 0;
    // virtual uint32_t getSpecificMetaRequest() = 0;
};
}
}
#endif  // CLIENT_ICLIENTOUTGOING_HPP_