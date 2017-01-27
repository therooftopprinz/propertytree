#include <client/src/LocalPTree.hpp>

namespace ptree
{
namespace client
{

LocalPTree::LocalPTree(IClientOutgoing& outgoing, TransactionsCV& transactionsCV):
    outgoing(outgoing), transactionsCV(transactionsCV), log("LocalPTree")
{
}

ValueContainerPtr LocalPTree::createValue(std::string path, Buffer& value)
{
    auto created  = outgoing.createRequest(path, protocol::PropertyType::Value, value);
    if (transactionsCV.waitTransactionCV(created.first))
    {
        protocol::CreateResponse response;
        response.unpackFrom(created.second->getBuffer());
        if ( response.response  == protocol::CreateResponse::Response::OK)
        {
            log << logger::DEBUG << "VALUE CREATED WITH UUID " << response.uuid;
            auto vc = std::make_shared<ValueContainer>(response.uuid, path, value, true);
            /**TODO: push vc on property map**/
            return vc;
        }
        else
        {
            log << logger::ERROR << "VALUE CREATE REQUEST NOT OK";
        }
    }
    else
    {
        log << logger::ERROR << "VALUE CREATE REQUEST TIMEOUT";
    }
    return ValueContainerPtr();
}

NodeContainerPtr LocalPTree::createNode(std::string path)
{
    Buffer empty;
    auto created  = outgoing.createRequest(path, protocol::PropertyType::Node, empty);
    if (transactionsCV.waitTransactionCV(created.first))
    {
        protocol::CreateResponse response;
        response.unpackFrom(created.second->getBuffer());
        if ( response.response  == protocol::CreateResponse::Response::OK)
        {
            log << logger::DEBUG << "NODE CREATED WITH UUID " << response.uuid;
            auto vc = std::make_shared<NodeContainer>(response.uuid, path, true);
            /**TODO: push vc on property map**/
            return vc;
        }
        else
        {
            log << logger::ERROR << "NODE CREATE REQUEST NOT OK";
        }
    }
    else
    {
        log << logger::ERROR << "NODE CREATE REQUEST TIMEOUT";
    }
    return NodeContainerPtr();
}

// RpcContainerPtr LocalPTree::createRpc(std::string, std::function<Buffer(Buffer&)>, std::function<void(Buffer&)>)
// {
// }

// bool LocalPTree::createNode(std::string)
// {
// }

// ValueContainerPtr LocalPTree::getValue(std::string&)
// {
// }

// RpcContainerPtr LocalPTree::getRpc(std::string&)
// {
// }

}
}
