
#ifndef INTERFACE_PROTOOCOLX_HPP_
#define INTERFACE_PROTOOCOLX_HPP_

#include <cstdint>
#include <memory>
#include "MessageEssential.hpp"

namespace ptree
{

namespace protocol
{

using protocol_x::Uuid;
using protocol_x::PropertyType;
using protocol_x::MessageType;
using protocol_x::MessageHeader;

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
};

struct PropertyUpdateNotificationEntry
{
    PropertyUpdateNotificationEntry(){}
    PropertyUpdateNotificationEntry(Uuid uuid, Buffer& buffer):
        uuid(uuid), data(buffer) {}
    Simple<Uuid> uuid;
    BufferBlock data;
    MESSAGE_FIELDS(uuid, data);
};


struct PropertyUpdateNotification
{
    BlockArray<PropertyUpdateNotificationEntry> propertyUpdateNotifications;
    MESSAGE_FIELDS(propertyUpdateNotifications);
};


struct UnsubscribePropertyUpdateRequest
{
    Simple<Uuid> uuid;
    MESSAGE_FIELDS(uuid);
};

struct UnsubscribePropertyUpdateResponse
{
    enum class Response : uint8_t {OK, NOT_SUBSCRIBED, NOT_A_VALUE, UUID_NOT_FOUND};
    Simple<Response> response;
    MESSAGE_FIELDS(response);
};

struct GetValueRequest
{
    Simple<Uuid> uuid;
    MESSAGE_FIELDS(uuid);
};

struct GetValueResponse
{
    BufferBlock data;
    MESSAGE_FIELDS(data);
};


struct RpcRequest
{
    Simple<Uuid> uuid;
    BufferBlock parameter;
    MESSAGE_FIELDS(uuid, parameter);
};

struct RpcResponse
{
    BufferBlock returnValue;
    MESSAGE_FIELDS(returnValue);
};

struct HandleRpcRequest
{
    Simple<uint64_t> callerId;
    Simple<uint32_t> callerTransactionId;
    Simple<Uuid> uuid;
    BufferBlock parameter;
    MESSAGE_FIELDS(callerId, callerTransactionId, uuid, parameter);
};

struct HandleRpcResponse
{
    Simple<uint64_t> callerId;
    Simple<uint32_t> callerTransactionId;
    BufferBlock returnValue;
    MESSAGE_FIELDS(callerId, callerTransactionId, returnValue);
};


struct GetSpecificMetaRequest
{
    String path;
    MESSAGE_FIELDS(path);
};

struct GetSpecificMetaResponse
{
    MetaCreate meta;
    MESSAGE_FIELDS(meta);
};

//////////////////

typedef std::shared_ptr<MessageHeader> MessageHeaderPtr;

} // namespace protocol
} // namespace ptree


// Notes:
// CreateResponse::response size

#endif  // INTERFACE_PROTOOCOL_HPP_