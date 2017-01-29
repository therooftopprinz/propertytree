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
    virtual std::pair<uint32_t,std::shared_ptr<TransactionCV>>
        createRequest(std::string path, protocol::PropertyType type, Buffer& value) = 0;
    virtual std::pair<uint32_t,std::shared_ptr<TransactionCV>>
        getValue(protocol::Uuid uuid) = 0;
    virtual std::pair<uint32_t,std::shared_ptr<TransactionCV>>
        getSpecificMeta(std::string& path) = 0;
    virtual std::pair<uint32_t,std::shared_ptr<TransactionCV>>
        subscribePropertyUpdate(protocol::Uuid uuid) = 0;
    virtual std::pair<uint32_t,std::shared_ptr<TransactionCV>>
        unsubscribePropertyUpdate(protocol::Uuid uuid) = 0;
    virtual void setValueIndication(protocol::Uuid uuid, Buffer&& data) = 0;
    // virtual uint32_t deleteRequest() = 0;
    // virtual uint32_t rpcRequest() = 0;
    // virtual uint32_t handleRpcRequest() = 0;
};
}
}
#endif  // CLIENT_ICLIENTOUTGOING_HPP_