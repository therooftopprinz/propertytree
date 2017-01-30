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
class IClientOutgoing
{
public:
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
    virtual void handleRpcResponse(uint32_t transactionId, protocol::Message& msg) = 0;
    virtual std::pair<uint32_t,std::shared_ptr<TransactionCV>>
        deleteRequest(protocol::Uuid) = 0;
    virtual std::pair<uint32_t,std::shared_ptr<TransactionCV>>
        rpcRequest(protocol::Uuid uuid, protocol::Buffer&& parameter) = 0;
protected:
};
}
}
#endif  // CLIENT_ICLIENTOUTGOING_HPP_