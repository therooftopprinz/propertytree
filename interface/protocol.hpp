#ifndef INTERFACE_PROTOOCOL_HPP_
#define INTERFACE_PROTOOCOL_HPP_

#include <cstdint>
#include <memory>
#include "MessageEssential.hpp"

#define PACKED __attribute__ ((packed))

namespace ptree
{

namespace protocol
{


#define GETTER_AND_PARSER \
inline std::vector<uint8_t> getPacked(){\
    Buffer enbuff(this->size());\
    protocol::BufferView enbuffv(enbuff);\
    protocol::Encoder en(enbuffv);\
    *this >> en;\
    return enbuff;}\
inline void unpackFrom(std::vector<uint8_t>& message){\
    protocol::Decoder de(message.data(), message.data()+message.size());\
    *this << de;}

typedef uint32_t Uuid;

enum class PropertyType : uint8_t
{
    Value,
    Node,
    Rpc
};

enum class MessageType : uint8_t
{
    SigninRequest = 42, // 2a
    SigninResponse, // 2b
    CreateRequest, // 2c
    CreateResponse, //2d
    MetaUpdateNotification, // 2e
    DeleteRequest, // 2f
    DeleteResponse, // 30
    SetValueIndication, // 31
    SubscribePropertyUpdateRequest, // 32 
    SubscribePropertyUpdateResponse, // 33 
    PropertyUpdateNotification, // 34
    UnsubscribePropertyUpdateRequest, // 35
    UnsubscribePropertyUpdateResponse, // 36
    GetValueRequest, // 37
    GetValueResponse, // 38
    RpcRequest, // 39
    RpcResponse, // 3a
    HandleRpcRequest, // 3b
    HandleRpcResponse, // 3c
    GetSpecificMetaRequest, // 3d
    GetSpecificMetaResponse, // 3e

    AquireOwnershipRequest,
    AquireOwnershipResponse,
    ReleaseOwnershipRequest,
    ReleaseOwnershipResponse
};

struct PACKED MessageHeader
{
    MessageType type;
    uint32_t size;
    uint32_t transactionId;
};


struct SigninRequest
{
    SigninRequest():
        featureFlag(0)
    {}
    enum class FeatureFlag : uint8_t
    {
        ENABLE_METAUPDATE = 0
    };

    Simple<uint32_t> version;
    Simple<uint32_t> refreshRate;
    Simple<uint64_t> featureFlag;

    inline void setFeature(FeatureFlag flag)
    {
        featureFlag |= 1 << (uint8_t)flag;
    }

    inline void resetFeature(FeatureFlag flag)
    {
        featureFlag &= ~(uint64_t(1) << (uint8_t)flag);
    }

    inline bool isSetFeature(FeatureFlag flag)
    {
        return featureFlag & uint64_t(1) << (uint8_t)flag;
    }

    MESSAGE_FIELDS(version, refreshRate, featureFlag);
    GETTER_AND_PARSER
};

struct MetaCreate;
struct SigninResponse
{
    Simple<uint32_t> version;
    MESSAGE_FIELDS(version);
    GETTER_AND_PARSER
};


struct CreateRequest
{
    Simple<PropertyType> type;
    BufferBlock data;
    String path;
    MESSAGE_FIELDS(type, data, path);
    GETTER_AND_PARSER
};

struct CreateResponse
{
    enum class Response : uint8_t {OK, PARENT_NOT_FOUND, MALFORMED_PATH, ALREADY_EXIST, TYPE_ERROR};
    Simple<Response> response;
    Simple<Uuid> uuid;
    MESSAGE_FIELDS(response, uuid);
    GETTER_AND_PARSER
};

struct MetaCreate
{
    MetaCreate()
    {}

    MetaCreate(Uuid uuid, PropertyType ptype, std::string path):
        uuid(uuid), propertyType(ptype), path(path)
    {}

    Simple<Uuid> uuid;
    Simple<PropertyType> propertyType;
    String path;
    MESSAGE_FIELDS(uuid, propertyType, path);
    GETTER_AND_PARSER
};

struct MetaDelete
{
    MetaDelete()
    {}

    MetaDelete(Uuid uuid):
        uuid(uuid)
    {}

    Simple<Uuid> uuid;
    MESSAGE_FIELDS(uuid);
    GETTER_AND_PARSER
};

struct MetaUpdateNotification
{
    BlockArray<MetaCreate> creations;
    BlockArray<MetaDelete> deletions;
    MESSAGE_FIELDS(creations, deletions);
    GETTER_AND_PARSER
};


struct DeleteRequest
{
    String path;
    MESSAGE_FIELDS(path);
    GETTER_AND_PARSER
};

struct DeleteResponse
{
    enum class Response : uint8_t {OK, OBJECT_NOT_FOUND, NOT_PERMITTED, NOT_EMPTY, MALFORMED_PATH};
    Simple<Response> response;
    MESSAGE_FIELDS(response);
    GETTER_AND_PARSER
};

struct SetValueIndication
{
    Simple<Uuid> uuid;
    BufferBlock data;
    MESSAGE_FIELDS(uuid, data);
    GETTER_AND_PARSER
};


struct SubscribePropertyUpdateRequest
{
    Simple<Uuid> uuid;
    MESSAGE_FIELDS(uuid);
    GETTER_AND_PARSER
};

struct SubscribePropertyUpdateResponse
{
    enum class Response : uint8_t {OK, UUID_NOT_FOUND, NOT_A_VALUE};
    Simple<Response> response;
    MESSAGE_FIELDS(response);
    GETTER_AND_PARSER
};

struct PropertyUpdateNotificationEntry
{
    PropertyUpdateNotificationEntry(){}
    PropertyUpdateNotificationEntry(Uuid uuid, Buffer& buffer):
        uuid(uuid), data(buffer) {}
    Simple<Uuid> uuid;
    BufferBlock data;
    MESSAGE_FIELDS(uuid, data);
    GETTER_AND_PARSER
};


struct PropertyUpdateNotification
{
    BlockArray<PropertyUpdateNotificationEntry> propertyUpdateNotifications;
    MESSAGE_FIELDS(propertyUpdateNotifications);
    GETTER_AND_PARSER
};


struct UnsubscribePropertyUpdateRequest
{
    Simple<Uuid> uuid;
    MESSAGE_FIELDS(uuid);
    GETTER_AND_PARSER
};

struct UnsubscribePropertyUpdateResponse
{
    enum class Response : uint8_t {OK, NOT_SUBSCRIBED, NOT_A_VALUE, UUID_NOT_FOUND};
    Simple<Response> response;
    MESSAGE_FIELDS(response);
    GETTER_AND_PARSER
};

struct GetValueRequest
{
    Simple<Uuid> uuid;
    MESSAGE_FIELDS(uuid);
    GETTER_AND_PARSER
};

struct GetValueResponse
{
    BufferBlock data;
    MESSAGE_FIELDS(data);
    GETTER_AND_PARSER
};


struct RpcRequest
{
    Simple<Uuid> uuid;
    BufferBlock parameter;
    MESSAGE_FIELDS(uuid, parameter);
    GETTER_AND_PARSER
};

struct RpcResponse
{
    BufferBlock returnValue;
    MESSAGE_FIELDS(returnValue);
    GETTER_AND_PARSER
};

struct HandleRpcRequest
{
    Simple<uint64_t> callerId;
    Simple<uint32_t> callerTransactionId;
    Simple<Uuid> uuid;
    BufferBlock parameter;
    MESSAGE_FIELDS(callerId, callerTransactionId, uuid, parameter);
    GETTER_AND_PARSER
};

struct HandleRpcResponse
{
    Simple<uint64_t> callerId;
    Simple<uint32_t> callerTransactionId;
    BufferBlock returnValue;
    MESSAGE_FIELDS(callerId, callerTransactionId, returnValue);
    GETTER_AND_PARSER
};


struct GetSpecificMetaRequest
{
    String path;
    MESSAGE_FIELDS(path);
    GETTER_AND_PARSER
};

struct GetSpecificMetaResponse
{
    MetaCreate meta;
    MESSAGE_FIELDS(meta);
    GETTER_AND_PARSER
};

//////////////////

typedef std::shared_ptr<MessageHeader> MessageHeaderPtr;

} // namespace protocol
} // namespace ptree


// Notes:
// CreateResponse::response size

#endif  // INTERFACE_PROTOOCOL_HPP_