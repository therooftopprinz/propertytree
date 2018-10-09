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

void ClientOutgoing::sendToServer(uint32_t tid, protocol::MessageType mtype, protocol::Message& msg)
{
    std::lock_guard<std::mutex> sendGuard(sendLock);

    Buffer header = createHeader(mtype, msg.size(), tid);
    endpoint.send(header.data(), header.size());

    Buffer responseMessageBuffer = msg.getPacked();
    log << logger::DEBUG << "sendToServer(" << uint32_t(mtype) << ", "
        << 9+responseMessageBuffer.size() << ", " << tid << "): " << msg.toString();
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
    auto tcv = transactionsCV.addTransactionCV(tid);
    sendToServer(tid, protocol::MessageType::SigninRequest, signIn);
    return std::make_pair(tid, tcv);
}

std::pair<uint32_t,std::shared_ptr<TransactionCV>>
    ClientOutgoing::createRequest(const std::string& path, protocol::PropertyType type, Buffer& value)
{
    protocol::CreateRequest request;
    request.path = path;
    request.data = value;
    request.type = type;
    auto tid = transactionIdGenerator.get();
    auto tcv = transactionsCV.addTransactionCV(tid);
    sendToServer(tid, protocol::MessageType::CreateRequest, request);
    return std::make_pair(tid, tcv);
}

std::pair<uint32_t,std::shared_ptr<TransactionCV>>
    ClientOutgoing::getValue(protocol::Uuid uuid)
{
    protocol::GetValueRequest request;
    request.uuid = uuid;
    auto tid = transactionIdGenerator.get();
    auto tcv = transactionsCV.addTransactionCV(tid);
    sendToServer(tid, protocol::MessageType::GetValueRequest, request);
    return std::make_pair(tid, tcv);
}

std::pair<uint32_t,std::shared_ptr<TransactionCV>>
    ClientOutgoing::getSpecificMeta(const std::string& path)
{
    protocol::GetSpecificMetaRequest request;
    request.path = path;
    auto tid = transactionIdGenerator.get();
    auto tcv = transactionsCV.addTransactionCV(tid);
    sendToServer(tid, protocol::MessageType::GetSpecificMetaRequest, request);
    return std::make_pair(tid, tcv);
}

std::pair<uint32_t,std::shared_ptr<TransactionCV>>
    ClientOutgoing::subscribePropertyUpdate(protocol::Uuid uuid)
{
    protocol::SubscribePropertyUpdateRequest request;
    request.uuid = uuid;
    auto tid = transactionIdGenerator.get();
    auto tcv = transactionsCV.addTransactionCV(tid);
    sendToServer(tid, protocol::MessageType::SubscribePropertyUpdateRequest, request);
    return std::make_pair(tid, tcv);
}

std::pair<uint32_t,std::shared_ptr<TransactionCV>>
    ClientOutgoing::unsubscribePropertyUpdate(protocol::Uuid uuid)
{
    protocol::UnsubscribePropertyUpdateRequest request;
    request.uuid = uuid;
    auto tid = transactionIdGenerator.get();
    auto tcv = transactionsCV.addTransactionCV(tid);
    sendToServer(tid, protocol::MessageType::UnsubscribePropertyUpdateRequest, request);
    return std::make_pair(tid, tcv);
}

void ClientOutgoing::setValueIndication(protocol::Uuid uuid, Buffer&& data)
{
    protocol::SetValueIndication indication;
    indication.uuid = uuid;
    indication.data = std::move(data);
    auto tid = transactionIdGenerator.get();
    sendToServer(tid, protocol::MessageType::SetValueIndication, indication);
}

void ClientOutgoing::setValueIndication(protocol::Uuid uuid, Buffer& data)
{
    protocol::SetValueIndication indication;
    indication.uuid = uuid;
    indication.data = data;
    auto tid = transactionIdGenerator.get();
    sendToServer(tid, protocol::MessageType::SetValueIndication, indication);
}


void ClientOutgoing::handleRpcResponse(uint32_t transactionId, protocol::Message& msg)
{
    sendToServer(transactionId, protocol::MessageType::HandleRpcResponse, msg);
}

std::pair<uint32_t,std::shared_ptr<TransactionCV>>
    ClientOutgoing::deleteRequest(protocol::Uuid uuid)
{
    protocol::DeleteRequest request;
    request.uuid = uuid;
    auto tid = transactionIdGenerator.get();
    auto tcv = transactionsCV.addTransactionCV(tid);
    sendToServer(tid, protocol::MessageType::DeleteRequest, request);
    return std::make_pair(tid, tcv);
}

std::pair<uint32_t,std::shared_ptr<TransactionCV>>
    ClientOutgoing::rpcRequest(protocol::Uuid uuid, protocol::Buffer&& parameter)
{
    protocol::RpcRequest request;
    request.uuid = uuid;
    request.parameter = std::move(parameter);
    auto tid = transactionIdGenerator.get();
    auto tcv = transactionsCV.addTransactionCV(tid);
    sendToServer(tid, protocol::MessageType::RpcRequest, request);
    return std::make_pair(tid, tcv);
}

} // namespace client
} // namespace ptree