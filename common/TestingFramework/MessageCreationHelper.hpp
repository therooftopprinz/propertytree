#include <interface/protocol.hpp>
#include <list>

namespace ptree
{
namespace common
{

typedef std::vector<uint8_t> Buffer;

/*** Remove inliners ***/
struct MessageCreationHelper
{
    inline Buffer createHeader(protocol::MessageType type, uint32_t size, uint32_t transactionId)
    {
        Buffer header(sizeof(protocol::MessageHeader));
        protocol::MessageHeader& headerRaw = *((protocol::MessageHeader*)header.data());
        headerRaw.type = type;
        headerRaw.size = size;
        headerRaw.transactionId = transactionId;
        return header;
    }

    inline Buffer createSigninRequestMessage(uint32_t transactionId, uint32_t version, uint32_t refreshRate)
    {
        protocol_x::SigninRequest signin;
        signin.version = version;
        signin.refreshRate = refreshRate;
        signin.setFeature(protocol_x::SigninRequest::FeatureFlag::ENABLE_METAUPDATE);

        uint32_t sz = signin.size() + sizeof(protocol::MessageHeader);

        Buffer message = createHeader(protocol::MessageType::SigninRequest, sz, transactionId);
        auto enbuff = signin.getPacked();
        message.insert(message.end(), enbuff.begin(), enbuff.end());

        return message;
    }

    inline Buffer createSigninResponseMessage(uint32_t transactionId, uint32_t version)
    {
        protocol_x::SigninResponse signin;
        signin.version = version;

        uint32_t sz = signin.size() + sizeof(protocol::MessageHeader);

        Buffer message = createHeader(protocol::MessageType::SigninResponse, sz, transactionId);
        auto enbuff = signin.getPacked();
        message.insert(message.end(), enbuff.begin(), enbuff.end());

        return message;
    }

    inline Buffer createCreateRequestMessage(uint32_t transactionId, Buffer valueContainer, protocol::PropertyType type,
        std::string path)
    {
        protocol_x::CreateRequest createReq;
        createReq.type = type;
        createReq.data = valueContainer;
        createReq.path = path;

        uint32_t sz = createReq.size() + sizeof(protocol::MessageHeader);

        Buffer message = createHeader(protocol::MessageType::CreateRequest, sz, transactionId);
        Buffer enbuff = createReq.getPacked();
        message.insert(message.end(), enbuff.begin(), enbuff.end());

        return message;
    }

    inline Buffer createDeleteRequestMessage(uint32_t transactionId, std::string path)
    {
        protocol_x::DeleteRequest deleteReq;
        deleteReq.path = path;

        uint32_t sz = deleteReq.size() + sizeof(protocol::MessageHeader);

        Buffer message = createHeader(protocol::MessageType::DeleteRequest, sz, transactionId);
        Buffer enbuff = deleteReq.getPacked();
        message.insert(message.end(), enbuff.begin(), enbuff.end());

        return message;
    }

    inline Buffer createSetValueIndicationMessage(uint32_t transactionId, protocol::Uuid uuid, Buffer value)
    {
        protocol::SetValueIndication setval;
        setval.uuid = uuid;
        setval.data = value;
        uint32_t sz = setval.size() + sizeof(protocol::MessageHeader);

        Buffer message = createHeader(protocol::MessageType::SetValueIndication, sz, transactionId);
        Buffer enbuff(setval.size());
        protocol::BufferView enbuffv(enbuff);
        protocol::Encoder en(enbuffv);
        setval >> en;
        message.insert(message.end(), enbuff.begin(), enbuff.end());

        return message;
    }

    inline Buffer createSubscribePropertyUpdateRequestMessage(uint32_t transactionId, protocol::Uuid uuid)
    {
        protocol::SubscribePropertyUpdateRequest request;
        request.uuid = uuid;
        uint32_t sz = request.size() + sizeof(protocol::MessageHeader);

        Buffer message = createHeader(protocol::MessageType::SubscribePropertyUpdateRequest, sz, transactionId);
        Buffer enbuff(request.size());
        protocol::BufferView enbuffv(enbuff);
        protocol::Encoder en(enbuffv);
        request >> en;
        message.insert(message.end(), enbuff.begin(), enbuff.end());

        return message;
    }

    inline Buffer createUnsubscribePropertyUpdateRequestMessage(uint32_t transactionId, protocol::Uuid uuid)
    {
        protocol::UnsubscribePropertyUpdateRequest request;
        request.uuid = uuid;
        uint32_t sz = request.size() + sizeof(protocol::MessageHeader);

        Buffer message = createHeader(protocol::MessageType::UnsubscribePropertyUpdateRequest, sz, transactionId);
        Buffer enbuff(request.size());
        protocol::BufferView enbuffv(enbuff);
        protocol::Encoder en(enbuffv);
        request >> en;
        message.insert(message.end(), enbuff.begin(), enbuff.end());

        return message;
    }

    template<typename TT, protocol::MessageType TR, typename T>
    inline Buffer createCommonResponse(uint32_t transactionId, T response)
    {
        TT responseMsg;
        responseMsg.response = response;

        uint32_t sz = responseMsg.size() + sizeof(protocol::MessageHeader);

        Buffer message = createHeader(TR, sz, transactionId);
        Buffer enbuff = responseMsg.getPacked();
        message.insert(message.end(), enbuff.begin(), enbuff.end());

        return message;
    }

    inline Buffer createCreateResponseMessage(uint32_t transactionId, protocol_x::CreateResponse::Response response,
        protocol::Uuid uuid)
    {
        protocol_x::CreateResponse responseMsg;
        responseMsg.response = response;
        responseMsg.uuid = uuid;

        uint32_t sz = responseMsg.size() + sizeof(protocol::MessageHeader);

        Buffer message = createHeader(protocol::MessageType::CreateResponse , sz, transactionId);
        Buffer enbuff = responseMsg.getPacked();
        message.insert(message.end(), enbuff.begin(), enbuff.end());

        return message;
    }


    inline Buffer createGetValueRequestMessage(uint32_t transactionId, protocol::Uuid uuid)
    {
        protocol::GetValueRequest request;
        request.uuid = uuid;

        uint32_t sz = request.size() + sizeof(protocol::MessageHeader);

        Buffer message = createHeader(protocol::MessageType::GetValueRequest, sz, transactionId);
        Buffer enbuff(request.size());
        protocol::BufferView enbuffv(enbuff);
        protocol::Encoder en(enbuffv);
        request >> en;
        message.insert(message.end(), enbuff.begin(), enbuff.end());

        return message;
    }

    inline Buffer createGetValueResponseMessage(uint32_t transactionId, Buffer value)
    {
        protocol::GetValueResponse response;
        response.data = value;

        uint32_t sz = response.size() + sizeof(protocol::MessageHeader);

        Buffer message = createHeader(protocol::MessageType::GetValueResponse, sz, transactionId);
        Buffer enbuff(response.size());
        protocol::BufferView enbuffv(enbuff);
        protocol::Encoder en(enbuffv);
        response >> en;
        message.insert(message.end(), enbuff.begin(), enbuff.end());

        return message;
    }

    inline Buffer createRpcRequestMessage(uint32_t transactionId, protocol::Uuid uuid, Buffer parameter)
    {
        protocol::RpcRequest request;
        request.uuid = uuid;
        request.parameter = parameter;

        uint32_t sz = request.size() + sizeof(protocol::MessageHeader);

        Buffer message = createHeader(protocol::MessageType::RpcRequest, sz, transactionId);
        Buffer enbuff(request.size());
        protocol::BufferView enbuffv(enbuff);
        protocol::Encoder en(enbuffv);
        request >> en;
        message.insert(message.end(), enbuff.begin(), enbuff.end());

        return message;
    }

    inline Buffer createHandleRpcRequestMessage(uint32_t transactionId, uint64_t callerId, uint32_t callerTransactionId, protocol::Uuid uuid, Buffer parameter)
    {
        protocol::HandleRpcRequest request;
        request.callerId = callerId;
        request.callerTransactionId = callerTransactionId;
        request.uuid = uuid;
        request.parameter = parameter;

        uint32_t sz = request.size() + sizeof(protocol::MessageHeader);

        Buffer message = createHeader(protocol::MessageType::HandleRpcRequest, sz, transactionId);
        Buffer enbuff(request.size());
        protocol::BufferView enbuffv(enbuff);
        protocol::Encoder en(enbuffv);
        request >> en;
        message.insert(message.end(), enbuff.begin(), enbuff.end());

        return message;
    }

    inline Buffer createHandleRpcResponseMessage(uint32_t transactionId, uint64_t callerId, uint32_t callerTransactionId, Buffer returnValue)
    {
        protocol::HandleRpcResponse response;
        response.returnValue = returnValue;
        response.callerId = callerId;
        response.callerTransactionId = callerTransactionId;

        uint32_t sz = response.size() + sizeof(protocol::MessageHeader);

        Buffer message = createHeader(protocol::MessageType::HandleRpcResponse, sz, transactionId);
        Buffer enbuff(response.size());
        protocol::BufferView enbuffv(enbuff);
        protocol::Encoder en(enbuffv);
        response >> en;
        message.insert(message.end(), enbuff.begin(), enbuff.end());

        return message;
    }

    inline Buffer createRpcResponseMessage(uint32_t transactionId, Buffer returnValue)
    {
        protocol::RpcResponse response;
        response.returnValue = returnValue;

        uint32_t sz = response.size() + sizeof(protocol::MessageHeader);

        Buffer message = createHeader(protocol::MessageType::RpcResponse, sz, transactionId);
        Buffer enbuff(response.size());
        protocol::BufferView enbuffv(enbuff);
        protocol::Encoder en(enbuffv);
        response >> en;
        message.insert(message.end(), enbuff.begin(), enbuff.end());

        return message;
    }

    inline Buffer createGetSpecificMetaRequestMessage(uint32_t transactionId, std::string path)
    {
        protocol::GetSpecificMetaRequest request;
        request.path = path;

        uint32_t sz = request.size() + sizeof(protocol::MessageHeader);

        Buffer message = createHeader(protocol::MessageType::GetSpecificMetaRequest, sz, transactionId);
        Buffer enbuff(request.size());
        protocol::BufferView enbuffv(enbuff);
        protocol::Encoder en(enbuffv);
        request >> en;
        message.insert(message.end(), enbuff.begin(), enbuff.end());

        return message;
    }

    inline Buffer createGetSpecificMetaResponseMessage(uint32_t transactionId, protocol::Uuid uuid,
        protocol::PropertyType ptype, std::string path)
    {
        protocol::GetSpecificMetaResponse response;
        response.meta = protocol::MetaCreate(uuid, ptype, path);

        uint32_t sz = response.size() + sizeof(protocol::MessageHeader);

        Buffer message = createHeader(protocol::MessageType::GetSpecificMetaResponse, sz, transactionId);
        Buffer enbuff(response.size());
        protocol::BufferView enbuffv(enbuff);
        protocol::Encoder en(enbuffv);
        response >> en;

        message.insert(message.end(), enbuff.begin(), enbuff.end());

        return message;
    }

    inline Buffer createPropertyUpdateNotificationMessage(uint32_t transactionId,
        std::list<protocol::PropertyUpdateNotificationEntry> updates)
    {
        protocol::PropertyUpdateNotification notif;

        for (auto& i : updates)
        {
            notif.propertyUpdateNotifications->push_back(i);
        }

        uint32_t sz = notif.size() + sizeof(protocol::MessageHeader);

        Buffer message = createHeader(protocol::MessageType::PropertyUpdateNotification, sz, transactionId);
        Buffer enbuff(notif.size());
        protocol::BufferView enbuffv(enbuff);
        protocol::Encoder en(enbuffv);
        notif >> en;

        message.insert(message.end(), enbuff.begin(), enbuff.end());

        return message;
    }


    inline Buffer createMetaUpdateNotificationMessage(uint32_t transactionId,
        std::list<protocol_x::MetaCreate> creates, std::list<protocol_x::MetaDelete> deletes)
    {
        protocol_x::MetaUpdateNotification notif;

        for (auto& i : creates)
        {
            notif.creations.get().emplace_back(i);
        }

        for (auto& i : deletes)
        {
            notif.deletions.get().emplace_back(i);
        }


        uint32_t sz = notif.size() + sizeof(protocol::MessageHeader);

        Buffer message = createHeader(protocol::MessageType::MetaUpdateNotification, sz, transactionId);
        Buffer enbuff = notif.getPacked();
        message.insert(message.end(), enbuff.begin(), enbuff.end());

        return message;
    }
};

}
}