#include "PTreeClient.hpp"
#include "MessageHandlerFactory.hpp"

namespace ptree
{
namespace client
{

PTreeClient::PTreeClient(common::IEndPointPtr endpoint):
    endpoint(endpoint),
    outgoing(transactionsCV, *this->endpoint),
    ptree(outgoing, transactionsCV),
    incoming(transactionsCV, *this->endpoint, ptree, outgoing),
    log("PTreeClient")
{
    std::list<protocol::SigninRequest::FeatureFlag> ft;
    ft.push_back(protocol::SigninRequest::FeatureFlag::ENABLE_METAUPDATE);
    auto trans = outgoing.signinRequest(200*1000, ft);

    if (transactionsCV.waitTransactionCV(trans.first))
    {
        log << logger::DEBUG << "signin response received.";

        protocol::SigninResponse response;
        response.unpackFrom(trans.second->getBuffer());
    }
    else
    {
        log << logger::ERROR << "SIGNIN TIMEOUT";
    }
}

PTreeClient::~PTreeClient()
{
    log << logger::DEBUG << "~PTreeClient";
}

LocalPTreePtr PTreeClient::getPTree()
{
    return std::shared_ptr<LocalPTree>(shared_from_this(), &ptree);
}

}
}
