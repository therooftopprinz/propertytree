#ifndef INTERFACE_PROTOOCOL_HPP_
#define INTERFACE_PROTOOCOL_HPP_

#include <cstdint>
#include <memory>
#include "NewMessageCoDec.hpp"
#include "MessageCoDec.hpp"
#include "MessageCoDecHelpers.hpp"

namespace ptree
{

namespace protocol
{

#define PACKED __attribute__ ((packed))

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

struct Message
{
    virtual ~Message() {}
    virtual std::vector<uint8_t> getPacked() = 0;
    virtual bool unpackFrom(std::vector<uint8_t>& message) = 0;
    virtual std::string toString() = 0;
    virtual uint32_t size() = 0;
};

struct SigninRequest: public Message
{
    SigninRequest():
        featureFlag(0)
    {}
    enum class FeatureFlag : uint8_t
    {
        ENABLE_METAUPDATE = 0
    };

    uint32_t version;
    uint32_t refreshRate;
    uint64_t featureFlag;

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

    MESSAGE_FIELDS(version,refreshRate,featureFlag);
};

struct SigninResponse: public Message
{
    uint32_t version;
    MESSAGE_FIELDS(version);
};

struct CreateRequest: public Message
{
    PropertyType type;
    std::vector<uint8_t> data;
    std::string path;
    MESSAGE_FIELDS(type,data,path);
};

struct CreateResponse: public Message
{
    enum class Response : uint8_t {OK, PARENT_NOT_FOUND, MALFORMED_PATH, ALREADY_EXIST, TYPE_ERROR};
    Response response;
    Uuid uuid;
    MESSAGE_FIELDS(response, uuid);
};

struct MetaCreate : public BlockBase
{
    MetaCreate()
    {}

    MetaCreate(Uuid uuid, PropertyType ptype, std::string path):
        uuid(uuid), propertyType(ptype), path(path)
    {}

    Uuid uuid;
    PropertyType propertyType;
    std::string path;
    MESSAGE_FIELDS(uuid,propertyType,path);
};

struct MetaDelete : public BlockBase
{
    MetaDelete()
    {}

    MetaDelete(Uuid uuid):
        uuid(uuid)
    {}

    Uuid uuid;
    MESSAGE_FIELDS(uuid);
};

struct MetaUpdateNotification: public Message
{
    BlockArray<MetaCreate> creations;
    BlockArray<MetaDelete> deletions;
    MESSAGE_FIELDS(BLOCK creations,BLOCK deletions);
};

struct DeleteRequest: public Message
{
    protocol::Uuid uuid;
    MESSAGE_FIELDS(uuid);
};

struct DeleteResponse: public Message
{
    enum class Response : uint8_t {OK, OBJECT_NOT_FOUND, NOT_PERMITTED, NOT_EMPTY, MALFORMED_PATH};
    Response response;
    MESSAGE_FIELDS(response);
};

struct SetValueIndication: public Message
{
    Uuid uuid;
    std::vector<uint8_t> data;
    MESSAGE_FIELDS(uuid,data);
};

struct SubscribePropertyUpdateRequest: public Message
{
    Uuid uuid;
    MESSAGE_FIELDS(uuid);
};

struct SubscribePropertyUpdateResponse: public Message
{
    enum class Response : uint8_t {OK, UUID_NOT_FOUND, NOT_A_VALUE};
    Response response;
    MESSAGE_FIELDS(response);
};

struct PropertyUpdateNotificationEntry: public Message
{
    PropertyUpdateNotificationEntry(){}
    PropertyUpdateNotificationEntry(Uuid uuid, Buffer& buffer):
        uuid(uuid), data(buffer) {}
    Uuid uuid;
    std::vector<uint8_t> data;
    MESSAGE_FIELDS(uuid,data);
};

struct PropertyUpdateNotification: public Message
{
    BlockArray<PropertyUpdateNotificationEntry> propertyUpdateNotifications;
    MESSAGE_FIELDS(BLOCK propertyUpdateNotifications);
};


struct UnsubscribePropertyUpdateRequest: public Message
{
    Uuid uuid;
    MESSAGE_FIELDS(uuid);
};

struct UnsubscribePropertyUpdateResponse: public Message
{
    enum class Response : uint8_t {OK, NOT_SUBSCRIBED, NOT_A_VALUE, UUID_NOT_FOUND};
    Response response;
    MESSAGE_FIELDS(response);
};

struct GetValueRequest: public Message
{
    Uuid uuid;
    MESSAGE_FIELDS(uuid);
};

struct GetValueResponse: public Message
{
    std::vector<uint8_t> data;
    MESSAGE_FIELDS(data);
};

struct RpcRequest: public Message
{
    Uuid uuid;
    std::vector<uint8_t> parameter;
    MESSAGE_FIELDS(uuid,parameter);
};

struct RpcResponse: public Message
{
    std::vector<uint8_t> returnValue;
    MESSAGE_FIELDS(returnValue);
};

struct HandleRpcRequest: public Message
{
    uint64_t callerId;
    uint32_t callerTransactionId;
    Uuid uuid;
    std::vector<uint8_t> parameter;
    MESSAGE_FIELDS(callerId,callerTransactionId,uuid,parameter);
};

struct HandleRpcResponse: public Message
{
    uint64_t callerId;
    uint32_t callerTransactionId;
    std::vector<uint8_t> returnValue;
    MESSAGE_FIELDS(callerId,callerTransactionId,returnValue);
};


struct GetSpecificMetaRequest: public Message
{
    std::string path;
    MESSAGE_FIELDS(path);
};

struct GetSpecificMetaResponse: public Message
{
    MetaCreate meta;
    MESSAGE_FIELDS(BLOCK meta);
};

typedef std::shared_ptr<MessageHeader> MessageHeaderPtr;

} // namespace protocol
} // namespace ptree

#endif  // INTERFACE_PROTOOCOL_HPP_
