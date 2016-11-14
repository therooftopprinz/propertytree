#ifndef INTERFACE_PROTOOCOL_HPP_
#define INTERFACE_PROTOOCOL_HPP_

#include <cstdint>
#include <memory>

#define PACKED __attribute__ ((packed))

namespace ptree
{

namespace protocol
{

/** It is really frustrating how I can't express some of these into a c++ data structure.
    Data serialization library might really be needed. Let's see.
**/

typedef uint32_t Uuid;

enum class PropertyType : uint8_t
{
    Value,
    Node,
    Rpc
};

enum class MessageType : uint8_t
{
    SignInRequest = 42, // 2a
    SignInResponse, // 2b
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
    GetValueRequest, // <-----------------
    GetValueResponse, // 
    RpcRequest,
    RpcResponse,
    HandleRpcRequest,
    HandleRpcResponse,

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


struct PACKED SignInRequest
{
    uint32_t version;
    uint32_t updateInterval;
};

struct PACKED SignInResponse
{
    uint32_t version;
};

/*
*    HEADER | VALUE_SIZE | PROPERTY_TYPE | VALUE | PATH | NULL
*             (uint32_t)     (uint8_t)       []     []     0
*/

struct PACKED CreateRequest
{
    uint32_t size; // size for the value
    PropertyType type;
    uint8_t data[]; // value_data + path + 0
};

/*
*    HEADER | CREATION_STATUS
*               (uint32_t)
*/

struct PACKED CreateResponse
{
    enum class Response : uint8_t {OK, PARENT_NOT_FOUND, MALFORMED_PATH, ALREADY_EXIST, TYPE_ERROR};
    Response response;
    uint32_t pass;  // unused
};

/*
*   HEADER | ( CREATE_OBJECT |     UUID     | PROPERTY_TYPE | PATH | NULL )...
*                (uint8_t)      (uint32_t)      (uint8_t)      []     0
*          , ( DELETE_OBJECT |    UUID     )...
*                (uint8_t)       (uint32_t)
*/

struct PACKED MetaUpdateNotification
{
    enum class UpdateType : uint8_t {CREATE_OBJECT, DELETE_OBJECT};
};

/*
*   HEADER  | PATH | NULL  
*              []     0
*/

struct PACKED DeleteRequest
{
    uint8_t path[1];
};

/*
*   HEADER  | DELETE_STATUS
*               (uint8_t)
*/

struct PACKED DeleteResponse
{
    enum class Response : uint8_t {OK, OBJECT_NOT_FOUND, NOT_PERMITTED, NOT_EMPTY};
    Response response;
};

/*
*   HEADER  |    UUID    | DATA | NULL  
*             (uint32_t)    []     0
*/

struct PACKED SetValueIndication
{
    Uuid uuid;
    uint8_t data[];
};

typedef std::shared_ptr<MessageHeader> MessageHeaderPtr;

/*
*   HEADER  |    UUID
*             (uint32_t)
*/

struct PACKED SubscribePropertyUpdateRequest
{
    Uuid uuid;
};

/*
*   HEADER | REPSONSE
*            (uint8_t)
*/

struct PACKED SubscribePropertyUpdateResponse
{
    enum class Response : uint8_t {OK, UUID_NOT_FOUND, NOT_A_VALUE};
    Response response;
};


/*
*   HEADER  | (    UUID    |   DATA   )...
*               (uint32_t)      []
*/

struct PACKED PropertyUpdateNotification
{
};

/*
*   HEADER  |    UUID
*             (uint32_t)
*/

struct PACKED UnsubscribePropertyUpdateRequest
{
    Uuid uuid;
};

/*
*   HEADER  |  REPSONSE
*              (uint8_t)
*/

struct PACKED UnsubscribePropertyUpdateResponse
{
    enum class Response : uint8_t {OK, NOT_SUBSCRIBED, NOT_A_VALUE, UUID_NOT_FOUND};
    Response response;
};

/*
*   HEADER | UUID
*
*/

struct PACKED GetValueRequest
{
    Uuid uuid;
};


/*
*   HEADER | DATA
*             []
*/

struct PACKED GetValueResponse
{
    Uuid uuid;
    uint8_t data[];
};


typedef std::shared_ptr<MessageHeader> MessageHeaderPtr;

} // namespace protocol
} // namespace ptree


// Notes:
// CreateResponse::response size

#endif  // INTERFACE_PROTOOCOL_HPP_