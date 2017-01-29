#include "ClientOutgoing.hpp"

namespace ptree
{
namespace client
{

ClientOutgoing::ClientOutgoing(TransactionsCV& transactionsCV, common::IEndPoint& endpoint):
    transactionsCV(transactionsCV),
    endpoint(endpoint),
    log("ClientOutgoing")
{
}

ClientOutgoing::~ClientOutgoing()
{
}

/**TODO: single send for header and content**/
Buffer ClientOutgoing::createHeader(protocol::MessageType type, uint32_t payloadSize, uint32_t transactionId)
{
    Buffer header(sizeof(protocol::MessageHeader));
    protocol::MessageHeader& headerRaw = *((protocol::MessageHeader*)header.data());
    headerRaw.type = type;
    headerRaw.size = payloadSize+sizeof(protocol::MessageHeader);
    headerRaw.transactionId = transactionId;
    return header;
}

void ClientOutgoing::sendToClient(uint32_t tid, protocol::MessageType mtype, protocol::Message& msg)
{
    std::lock_guard<std::mutex> sendGuard(sendLock);

    Buffer header = createHeader(mtype, msg.size(), tid);
    endpoint.send(header.data(), header.size());

    Buffer responseMessageBuffer = msg.getPacked();
    endpoint.send(responseMessageBuffer.data(), responseMessageBuffer.size());
}

std::pair<uint32_t,std::shared_ptr<TransactionCV>>
    ClientOutgoing::signinRequest(int refreshRate, const std::list<protocol::SigninRequest::FeatureFlag> features)
{
    protocol::SigninRequest signIn;
    signIn.version = 1;
    signIn.refreshRate = refreshRate;
    for (const auto& i : features)
    {
        signIn.setFeature(i);
    }
    auto tid = transactionIdGenerator.get();
    sendToClient(tid, protocol::MessageType::SigninRequest, signIn);
    auto tcv = transactionsCV.addTransactionCV(tid);
    return std::make_pair(tid, tcv);
}

std::pair<uint32_t,std::shared_ptr<TransactionCV>>
    ClientOutgoing::createRequest(std::string path, protocol::PropertyType type, Buffer& value)
{
    protocol::CreateRequest request;
    request.path = path;
    request.data = value;
    request.type = type;
    auto tid = transactionIdGenerator.get();
    sendToClient(tid, protocol::MessageType::CreateRequest, request);
    auto tcv = transactionsCV.addTransactionCV(tid);
    return std::make_pair(tid, tcv);
}

std::pair<uint32_t,std::shared_ptr<TransactionCV>>
    ClientOutgoing::getValue(protocol::Uuid uuid)
{
    protocol::GetValueRequest request;
    request.uuid = uuid;
    auto tid = transactionIdGenerator.get();
    sendToClient(tid, protocol::MessageType::GetValueRequest, request);
    auto tcv = transactionsCV.addTransactionCV(tid);
    return std::make_pair(tid, tcv);
}

std::pair<uint32_t,std::shared_ptr<TransactionCV>>
    ClientOutgoing::getSpecificMeta(std::string& path)
{
    protocol::GetSpecificMetaRequest request;
    request.path = path;
    auto tid = transactionIdGenerator.get();
    sendToClient(tid, protocol::MessageType::GetSpecificMetaRequest, request);
    auto tcv = transactionsCV.addTransactionCV(tid);
    return std::make_pair(tid, tcv);
}

}
}