// Type:  ('u64', {'type': 'unsigned'})
// Type:  ('u64', {'width': '64'})
// Type:  ('u32', {'type': 'unsigned'})
// Type:  ('u32', {'width': '32'})
// Type:  ('u16', {'type': 'unsigned'})
// Type:  ('u16', {'width': '16'})
// Type:  ('u8', {'type': 'unsigned'})
// Type:  ('u8', {'width': '8'})
// Type:  ('u64Array', {'type': 'u64'})
// Type:  ('u64Array', {'dynamic_array': ''})
// Type:  ('Buffer', {'type': 'unsigned'})
// Type:  ('Buffer', {'width': '8'})
// Type:  ('Buffer', {'dynamic_array': ''})
// Type:  ('String', {'type': 'asciiz'})
// Enumeration:  ('Cause', ('OK', None))
// Enumeration:  ('Cause', ('NOT_FOUND', None))
// Enumeration:  ('Cause', ('ALREADY_EXIST', None))
// Enumeration:  ('Cause', ('NOT_PERMITTED', None))
// Enumeration:  ('Cause', ('NO_HANDLER', None))
// Sequence:  NamedNode ('String', 'name')
// Sequence:  NamedNode ('u64', 'uuid')
// Sequence:  NamedNode ('u64', 'parentUuid')
// Type:  ('NamedNodeList', {'type': 'NamedNode'})
// Type:  ('NamedNodeList', {'dynamic_array': ''})
// Sequence:  SigninRequest ('u8', 'spare')
// Sequence:  SigninAccept ('u32', 'sessionId')
// Sequence:  CreateRequest ('String', 'name')
// Sequence:  CreateRequest ('u64', 'parentUuid')
// Sequence:  CreateAccept ('u64', 'uuid')
// Sequence:  CreateReject ('Cause', 'cause')
// Sequence:  GetRequest ('u64', 'uuid')
// Sequence:  GetAccept ('Buffer', 'data')
// Sequence:  GetReject ('Cause', 'cause')
// Sequence:  TreeInfoRequest ('u64', 'parentUuid')
// Sequence:  TreeInfoRequest ('String', 'name')
// Sequence:  TreeInfoRequest ('u8', 'recursive')
// Sequence:  TreeInfoResponse ('NamedNodeList', 'nodeToAddList')
// Sequence:  TreeInfoErrorResponse ('Cause', 'cause')
// Sequence:  TreeUpdateNotification ('NamedNodeList', 'nodeToAddList')
// Sequence:  TreeUpdateNotification ('u64Array', 'nodeToDelete')
// Sequence:  DeleteRequest ('u64', 'uuid')
// Sequence:  DeleteResponse ('Cause', 'cause')
// Sequence:  SetValueIndication ('u64', 'uuid')
// Sequence:  SetValueIndication ('Buffer', 'data')
// Sequence:  SubscribeRequest ('u64', 'uuid')
// Sequence:  SubscribeResponse ('Cause', 'cause')
// Sequence:  UnsubscribeRequest ('u64', 'uuid')
// Sequence:  UnsubscribeResponse ('Cause', 'cause')
// Sequence:  UpdateNotification ('u64', 'uuid')
// Sequence:  UpdateNotification ('Buffer', 'data')
// Sequence:  RpcRequest ('u64', 'uuid')
// Sequence:  RpcRequest ('Buffer', 'param')
// Sequence:  RpcAccept ('Buffer', 'value')
// Sequence:  RpcReject ('Cause', 'cause')
// Choice:  ('PropertyTreeMessages', 'SigninRequest')
// Choice:  ('PropertyTreeMessages', 'SigninAccept')
// Choice:  ('PropertyTreeMessages', 'CreateRequest')
// Choice:  ('PropertyTreeMessages', 'CreateAccept')
// Choice:  ('PropertyTreeMessages', 'CreateReject')
// Choice:  ('PropertyTreeMessages', 'GetRequest')
// Choice:  ('PropertyTreeMessages', 'GetAccept')
// Choice:  ('PropertyTreeMessages', 'GetReject')
// Choice:  ('PropertyTreeMessages', 'TreeInfoRequest')
// Choice:  ('PropertyTreeMessages', 'TreeInfoResponse')
// Choice:  ('PropertyTreeMessages', 'TreeInfoErrorResponse')
// Choice:  ('PropertyTreeMessages', 'TreeUpdateNotification')
// Choice:  ('PropertyTreeMessages', 'DeleteRequest')
// Choice:  ('PropertyTreeMessages', 'DeleteResponse')
// Choice:  ('PropertyTreeMessages', 'SetValueIndication')
// Choice:  ('PropertyTreeMessages', 'SubscribeRequest')
// Choice:  ('PropertyTreeMessages', 'SubscribeResponse')
// Choice:  ('PropertyTreeMessages', 'UnsubscribeRequest')
// Choice:  ('PropertyTreeMessages', 'UnsubscribeResponse')
// Choice:  ('PropertyTreeMessages', 'UpdateNotification')
// Choice:  ('PropertyTreeMessages', 'RpcRequest')
// Choice:  ('PropertyTreeMessages', 'RpcAccept')
// Choice:  ('PropertyTreeMessages', 'RpcReject')
// Sequence:  PropertyTreeMessage ('u16', 'transactionId')
// Sequence:  PropertyTreeMessage ('PropertyTreeMessages', 'message')
// Type:  ('PropertyTreeMessageArray', {'type': 'PropertyTreeMessage'})
// Type:  ('PropertyTreeMessageArray', {'dynamic_array': '256'})
// Choice:  ('PropertyTreeProtocol', 'PropertyTreeMessage')
// Choice:  ('PropertyTreeProtocol', 'PropertyTreeMessageArray')
// Generating for C++
#ifndef __CUM_MSG_HPP__
#define __CUM_MSG_HPP__
#include "cum/cum.hpp"
#include <optional>

/***********************************************
/
/            Message Definitions
/
************************************************/

using u64 = uint64_t;
using u32 = uint32_t;
using u16 = uint16_t;
using u8 = uint8_t;
using u64Array = cum::vector<u64, 4294967296>;
using Buffer = cum::vector<uint8_t, 4294967296>;
using String = std::string;
enum class Cause : uint8_t
{
    OK,
    NOT_FOUND,
    ALREADY_EXIST,
    NOT_PERMITTED,
    NO_HANDLER
};

struct NamedNode
{
    String name;
    u64 uuid;
    u64 parentUuid;
};

using NamedNodeList = cum::vector<NamedNode, 4294967296>;
struct SigninRequest
{
    u8 spare;
};

struct SigninAccept
{
    u32 sessionId;
};

struct CreateRequest
{
    String name;
    u64 parentUuid;
};

struct CreateAccept
{
    u64 uuid;
};

struct CreateReject
{
    Cause cause;
};

struct GetRequest
{
    u64 uuid;
};

struct GetAccept
{
    Buffer data;
};

struct GetReject
{
    Cause cause;
};

struct TreeInfoRequest
{
    u64 parentUuid;
    String name;
    u8 recursive;
};

struct TreeInfoResponse
{
    NamedNodeList nodeToAddList;
};

struct TreeInfoErrorResponse
{
    Cause cause;
};

struct TreeUpdateNotification
{
    NamedNodeList nodeToAddList;
    u64Array nodeToDelete;
};

struct DeleteRequest
{
    u64 uuid;
};

struct DeleteResponse
{
    Cause cause;
};

struct SetValueIndication
{
    u64 uuid;
    Buffer data;
};

struct SubscribeRequest
{
    u64 uuid;
};

struct SubscribeResponse
{
    Cause cause;
};

struct UnsubscribeRequest
{
    u64 uuid;
};

struct UnsubscribeResponse
{
    Cause cause;
};

struct UpdateNotification
{
    u64 uuid;
    Buffer data;
};

struct RpcRequest
{
    u64 uuid;
    Buffer param;
};

struct RpcAccept
{
    Buffer value;
};

struct RpcReject
{
    Cause cause;
};

using PropertyTreeMessages = std::variant<SigninRequest,SigninAccept,CreateRequest,CreateAccept,CreateReject,GetRequest,GetAccept,GetReject,TreeInfoRequest,TreeInfoResponse,TreeInfoErrorResponse,TreeUpdateNotification,DeleteRequest,DeleteResponse,SetValueIndication,SubscribeRequest,SubscribeResponse,UnsubscribeRequest,UnsubscribeResponse,UpdateNotification,RpcRequest,RpcAccept,RpcReject>;
struct PropertyTreeMessage
{
    u16 transactionId;
    PropertyTreeMessages message;
};

using PropertyTreeMessageArray = cum::vector<PropertyTreeMessage, 256>;
using PropertyTreeProtocol = std::variant<PropertyTreeMessage,PropertyTreeMessageArray>;
/***********************************************
/
/            Codec Definitions
/
************************************************/

inline void str(const char* pName, const Cause& pIe, std::string& pCtx, bool pIsLast)
{
    using namespace cum;
    if (pName)
    {
        pCtx = pCtx + "\"" + pName + "\":";
    }
    if (Cause::OK == pIe) pCtx += "\"OK\"";
    if (Cause::NOT_FOUND == pIe) pCtx += "\"NOT_FOUND\"";
    if (Cause::ALREADY_EXIST == pIe) pCtx += "\"ALREADY_EXIST\"";
    if (Cause::NOT_PERMITTED == pIe) pCtx += "\"NOT_PERMITTED\"";
    if (Cause::NO_HANDLER == pIe) pCtx += "\"NO_HANDLER\"";
    pCtx = pCtx + "}";
    if (!pIsLast)
    {
        pCtx += ",";
    }
}

inline void encode_per(const NamedNode& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    encode_per(pIe.name, pCtx);
    encode_per(pIe.uuid, pCtx);
    encode_per(pIe.parentUuid, pCtx);
}

inline void decode_per(NamedNode& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    decode_per(pIe.name, pCtx);
    decode_per(pIe.uuid, pCtx);
    decode_per(pIe.parentUuid, pCtx);
}

inline void str(const char* pName, const NamedNode& pIe, std::string& pCtx, bool pIsLast)
{
    using namespace cum;
    if (!pName)
    {
        pCtx = pCtx + "{";
    }
    else
    {
        pCtx = pCtx + "\"" + pName + "\":{";
    }
    size_t nOptional = 0;
    size_t nMandatory = 3;
    str("name", pIe.name, pCtx, !(--nMandatory+nOptional));
    str("uuid", pIe.uuid, pCtx, !(--nMandatory+nOptional));
    str("parentUuid", pIe.parentUuid, pCtx, !(--nMandatory+nOptional));
    pCtx = pCtx + "}";
    if (!pIsLast)
    {
        pCtx += ",";
    }
}

inline void encode_per(const SigninRequest& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    encode_per(pIe.spare, pCtx);
}

inline void decode_per(SigninRequest& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    decode_per(pIe.spare, pCtx);
}

inline void str(const char* pName, const SigninRequest& pIe, std::string& pCtx, bool pIsLast)
{
    using namespace cum;
    if (!pName)
    {
        pCtx = pCtx + "{";
    }
    else
    {
        pCtx = pCtx + "\"" + pName + "\":{";
    }
    size_t nOptional = 0;
    size_t nMandatory = 1;
    str("spare", pIe.spare, pCtx, !(--nMandatory+nOptional));
    pCtx = pCtx + "}";
    if (!pIsLast)
    {
        pCtx += ",";
    }
}

inline void encode_per(const SigninAccept& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    encode_per(pIe.sessionId, pCtx);
}

inline void decode_per(SigninAccept& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    decode_per(pIe.sessionId, pCtx);
}

inline void str(const char* pName, const SigninAccept& pIe, std::string& pCtx, bool pIsLast)
{
    using namespace cum;
    if (!pName)
    {
        pCtx = pCtx + "{";
    }
    else
    {
        pCtx = pCtx + "\"" + pName + "\":{";
    }
    size_t nOptional = 0;
    size_t nMandatory = 1;
    str("sessionId", pIe.sessionId, pCtx, !(--nMandatory+nOptional));
    pCtx = pCtx + "}";
    if (!pIsLast)
    {
        pCtx += ",";
    }
}

inline void encode_per(const CreateRequest& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    encode_per(pIe.name, pCtx);
    encode_per(pIe.parentUuid, pCtx);
}

inline void decode_per(CreateRequest& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    decode_per(pIe.name, pCtx);
    decode_per(pIe.parentUuid, pCtx);
}

inline void str(const char* pName, const CreateRequest& pIe, std::string& pCtx, bool pIsLast)
{
    using namespace cum;
    if (!pName)
    {
        pCtx = pCtx + "{";
    }
    else
    {
        pCtx = pCtx + "\"" + pName + "\":{";
    }
    size_t nOptional = 0;
    size_t nMandatory = 2;
    str("name", pIe.name, pCtx, !(--nMandatory+nOptional));
    str("parentUuid", pIe.parentUuid, pCtx, !(--nMandatory+nOptional));
    pCtx = pCtx + "}";
    if (!pIsLast)
    {
        pCtx += ",";
    }
}

inline void encode_per(const CreateAccept& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    encode_per(pIe.uuid, pCtx);
}

inline void decode_per(CreateAccept& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    decode_per(pIe.uuid, pCtx);
}

inline void str(const char* pName, const CreateAccept& pIe, std::string& pCtx, bool pIsLast)
{
    using namespace cum;
    if (!pName)
    {
        pCtx = pCtx + "{";
    }
    else
    {
        pCtx = pCtx + "\"" + pName + "\":{";
    }
    size_t nOptional = 0;
    size_t nMandatory = 1;
    str("uuid", pIe.uuid, pCtx, !(--nMandatory+nOptional));
    pCtx = pCtx + "}";
    if (!pIsLast)
    {
        pCtx += ",";
    }
}

inline void encode_per(const CreateReject& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    encode_per(pIe.cause, pCtx);
}

inline void decode_per(CreateReject& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    decode_per(pIe.cause, pCtx);
}

inline void str(const char* pName, const CreateReject& pIe, std::string& pCtx, bool pIsLast)
{
    using namespace cum;
    if (!pName)
    {
        pCtx = pCtx + "{";
    }
    else
    {
        pCtx = pCtx + "\"" + pName + "\":{";
    }
    size_t nOptional = 0;
    size_t nMandatory = 1;
    str("cause", pIe.cause, pCtx, !(--nMandatory+nOptional));
    pCtx = pCtx + "}";
    if (!pIsLast)
    {
        pCtx += ",";
    }
}

inline void encode_per(const GetRequest& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    encode_per(pIe.uuid, pCtx);
}

inline void decode_per(GetRequest& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    decode_per(pIe.uuid, pCtx);
}

inline void str(const char* pName, const GetRequest& pIe, std::string& pCtx, bool pIsLast)
{
    using namespace cum;
    if (!pName)
    {
        pCtx = pCtx + "{";
    }
    else
    {
        pCtx = pCtx + "\"" + pName + "\":{";
    }
    size_t nOptional = 0;
    size_t nMandatory = 1;
    str("uuid", pIe.uuid, pCtx, !(--nMandatory+nOptional));
    pCtx = pCtx + "}";
    if (!pIsLast)
    {
        pCtx += ",";
    }
}

inline void encode_per(const GetAccept& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    encode_per(pIe.data, pCtx);
}

inline void decode_per(GetAccept& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    decode_per(pIe.data, pCtx);
}

inline void str(const char* pName, const GetAccept& pIe, std::string& pCtx, bool pIsLast)
{
    using namespace cum;
    if (!pName)
    {
        pCtx = pCtx + "{";
    }
    else
    {
        pCtx = pCtx + "\"" + pName + "\":{";
    }
    size_t nOptional = 0;
    size_t nMandatory = 1;
    str("data", pIe.data, pCtx, !(--nMandatory+nOptional));
    pCtx = pCtx + "}";
    if (!pIsLast)
    {
        pCtx += ",";
    }
}

inline void encode_per(const GetReject& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    encode_per(pIe.cause, pCtx);
}

inline void decode_per(GetReject& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    decode_per(pIe.cause, pCtx);
}

inline void str(const char* pName, const GetReject& pIe, std::string& pCtx, bool pIsLast)
{
    using namespace cum;
    if (!pName)
    {
        pCtx = pCtx + "{";
    }
    else
    {
        pCtx = pCtx + "\"" + pName + "\":{";
    }
    size_t nOptional = 0;
    size_t nMandatory = 1;
    str("cause", pIe.cause, pCtx, !(--nMandatory+nOptional));
    pCtx = pCtx + "}";
    if (!pIsLast)
    {
        pCtx += ",";
    }
}

inline void encode_per(const TreeInfoRequest& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    encode_per(pIe.parentUuid, pCtx);
    encode_per(pIe.name, pCtx);
    encode_per(pIe.recursive, pCtx);
}

inline void decode_per(TreeInfoRequest& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    decode_per(pIe.parentUuid, pCtx);
    decode_per(pIe.name, pCtx);
    decode_per(pIe.recursive, pCtx);
}

inline void str(const char* pName, const TreeInfoRequest& pIe, std::string& pCtx, bool pIsLast)
{
    using namespace cum;
    if (!pName)
    {
        pCtx = pCtx + "{";
    }
    else
    {
        pCtx = pCtx + "\"" + pName + "\":{";
    }
    size_t nOptional = 0;
    size_t nMandatory = 3;
    str("parentUuid", pIe.parentUuid, pCtx, !(--nMandatory+nOptional));
    str("name", pIe.name, pCtx, !(--nMandatory+nOptional));
    str("recursive", pIe.recursive, pCtx, !(--nMandatory+nOptional));
    pCtx = pCtx + "}";
    if (!pIsLast)
    {
        pCtx += ",";
    }
}

inline void encode_per(const TreeInfoResponse& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    encode_per(pIe.nodeToAddList, pCtx);
}

inline void decode_per(TreeInfoResponse& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    decode_per(pIe.nodeToAddList, pCtx);
}

inline void str(const char* pName, const TreeInfoResponse& pIe, std::string& pCtx, bool pIsLast)
{
    using namespace cum;
    if (!pName)
    {
        pCtx = pCtx + "{";
    }
    else
    {
        pCtx = pCtx + "\"" + pName + "\":{";
    }
    size_t nOptional = 0;
    size_t nMandatory = 1;
    str("nodeToAddList", pIe.nodeToAddList, pCtx, !(--nMandatory+nOptional));
    pCtx = pCtx + "}";
    if (!pIsLast)
    {
        pCtx += ",";
    }
}

inline void encode_per(const TreeInfoErrorResponse& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    encode_per(pIe.cause, pCtx);
}

inline void decode_per(TreeInfoErrorResponse& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    decode_per(pIe.cause, pCtx);
}

inline void str(const char* pName, const TreeInfoErrorResponse& pIe, std::string& pCtx, bool pIsLast)
{
    using namespace cum;
    if (!pName)
    {
        pCtx = pCtx + "{";
    }
    else
    {
        pCtx = pCtx + "\"" + pName + "\":{";
    }
    size_t nOptional = 0;
    size_t nMandatory = 1;
    str("cause", pIe.cause, pCtx, !(--nMandatory+nOptional));
    pCtx = pCtx + "}";
    if (!pIsLast)
    {
        pCtx += ",";
    }
}

inline void encode_per(const TreeUpdateNotification& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    encode_per(pIe.nodeToAddList, pCtx);
    encode_per(pIe.nodeToDelete, pCtx);
}

inline void decode_per(TreeUpdateNotification& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    decode_per(pIe.nodeToAddList, pCtx);
    decode_per(pIe.nodeToDelete, pCtx);
}

inline void str(const char* pName, const TreeUpdateNotification& pIe, std::string& pCtx, bool pIsLast)
{
    using namespace cum;
    if (!pName)
    {
        pCtx = pCtx + "{";
    }
    else
    {
        pCtx = pCtx + "\"" + pName + "\":{";
    }
    size_t nOptional = 0;
    size_t nMandatory = 2;
    str("nodeToAddList", pIe.nodeToAddList, pCtx, !(--nMandatory+nOptional));
    str("nodeToDelete", pIe.nodeToDelete, pCtx, !(--nMandatory+nOptional));
    pCtx = pCtx + "}";
    if (!pIsLast)
    {
        pCtx += ",";
    }
}

inline void encode_per(const DeleteRequest& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    encode_per(pIe.uuid, pCtx);
}

inline void decode_per(DeleteRequest& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    decode_per(pIe.uuid, pCtx);
}

inline void str(const char* pName, const DeleteRequest& pIe, std::string& pCtx, bool pIsLast)
{
    using namespace cum;
    if (!pName)
    {
        pCtx = pCtx + "{";
    }
    else
    {
        pCtx = pCtx + "\"" + pName + "\":{";
    }
    size_t nOptional = 0;
    size_t nMandatory = 1;
    str("uuid", pIe.uuid, pCtx, !(--nMandatory+nOptional));
    pCtx = pCtx + "}";
    if (!pIsLast)
    {
        pCtx += ",";
    }
}

inline void encode_per(const DeleteResponse& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    encode_per(pIe.cause, pCtx);
}

inline void decode_per(DeleteResponse& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    decode_per(pIe.cause, pCtx);
}

inline void str(const char* pName, const DeleteResponse& pIe, std::string& pCtx, bool pIsLast)
{
    using namespace cum;
    if (!pName)
    {
        pCtx = pCtx + "{";
    }
    else
    {
        pCtx = pCtx + "\"" + pName + "\":{";
    }
    size_t nOptional = 0;
    size_t nMandatory = 1;
    str("cause", pIe.cause, pCtx, !(--nMandatory+nOptional));
    pCtx = pCtx + "}";
    if (!pIsLast)
    {
        pCtx += ",";
    }
}

inline void encode_per(const SetValueIndication& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    encode_per(pIe.uuid, pCtx);
    encode_per(pIe.data, pCtx);
}

inline void decode_per(SetValueIndication& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    decode_per(pIe.uuid, pCtx);
    decode_per(pIe.data, pCtx);
}

inline void str(const char* pName, const SetValueIndication& pIe, std::string& pCtx, bool pIsLast)
{
    using namespace cum;
    if (!pName)
    {
        pCtx = pCtx + "{";
    }
    else
    {
        pCtx = pCtx + "\"" + pName + "\":{";
    }
    size_t nOptional = 0;
    size_t nMandatory = 2;
    str("uuid", pIe.uuid, pCtx, !(--nMandatory+nOptional));
    str("data", pIe.data, pCtx, !(--nMandatory+nOptional));
    pCtx = pCtx + "}";
    if (!pIsLast)
    {
        pCtx += ",";
    }
}

inline void encode_per(const SubscribeRequest& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    encode_per(pIe.uuid, pCtx);
}

inline void decode_per(SubscribeRequest& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    decode_per(pIe.uuid, pCtx);
}

inline void str(const char* pName, const SubscribeRequest& pIe, std::string& pCtx, bool pIsLast)
{
    using namespace cum;
    if (!pName)
    {
        pCtx = pCtx + "{";
    }
    else
    {
        pCtx = pCtx + "\"" + pName + "\":{";
    }
    size_t nOptional = 0;
    size_t nMandatory = 1;
    str("uuid", pIe.uuid, pCtx, !(--nMandatory+nOptional));
    pCtx = pCtx + "}";
    if (!pIsLast)
    {
        pCtx += ",";
    }
}

inline void encode_per(const SubscribeResponse& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    encode_per(pIe.cause, pCtx);
}

inline void decode_per(SubscribeResponse& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    decode_per(pIe.cause, pCtx);
}

inline void str(const char* pName, const SubscribeResponse& pIe, std::string& pCtx, bool pIsLast)
{
    using namespace cum;
    if (!pName)
    {
        pCtx = pCtx + "{";
    }
    else
    {
        pCtx = pCtx + "\"" + pName + "\":{";
    }
    size_t nOptional = 0;
    size_t nMandatory = 1;
    str("cause", pIe.cause, pCtx, !(--nMandatory+nOptional));
    pCtx = pCtx + "}";
    if (!pIsLast)
    {
        pCtx += ",";
    }
}

inline void encode_per(const UnsubscribeRequest& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    encode_per(pIe.uuid, pCtx);
}

inline void decode_per(UnsubscribeRequest& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    decode_per(pIe.uuid, pCtx);
}

inline void str(const char* pName, const UnsubscribeRequest& pIe, std::string& pCtx, bool pIsLast)
{
    using namespace cum;
    if (!pName)
    {
        pCtx = pCtx + "{";
    }
    else
    {
        pCtx = pCtx + "\"" + pName + "\":{";
    }
    size_t nOptional = 0;
    size_t nMandatory = 1;
    str("uuid", pIe.uuid, pCtx, !(--nMandatory+nOptional));
    pCtx = pCtx + "}";
    if (!pIsLast)
    {
        pCtx += ",";
    }
}

inline void encode_per(const UnsubscribeResponse& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    encode_per(pIe.cause, pCtx);
}

inline void decode_per(UnsubscribeResponse& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    decode_per(pIe.cause, pCtx);
}

inline void str(const char* pName, const UnsubscribeResponse& pIe, std::string& pCtx, bool pIsLast)
{
    using namespace cum;
    if (!pName)
    {
        pCtx = pCtx + "{";
    }
    else
    {
        pCtx = pCtx + "\"" + pName + "\":{";
    }
    size_t nOptional = 0;
    size_t nMandatory = 1;
    str("cause", pIe.cause, pCtx, !(--nMandatory+nOptional));
    pCtx = pCtx + "}";
    if (!pIsLast)
    {
        pCtx += ",";
    }
}

inline void encode_per(const UpdateNotification& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    encode_per(pIe.uuid, pCtx);
    encode_per(pIe.data, pCtx);
}

inline void decode_per(UpdateNotification& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    decode_per(pIe.uuid, pCtx);
    decode_per(pIe.data, pCtx);
}

inline void str(const char* pName, const UpdateNotification& pIe, std::string& pCtx, bool pIsLast)
{
    using namespace cum;
    if (!pName)
    {
        pCtx = pCtx + "{";
    }
    else
    {
        pCtx = pCtx + "\"" + pName + "\":{";
    }
    size_t nOptional = 0;
    size_t nMandatory = 2;
    str("uuid", pIe.uuid, pCtx, !(--nMandatory+nOptional));
    str("data", pIe.data, pCtx, !(--nMandatory+nOptional));
    pCtx = pCtx + "}";
    if (!pIsLast)
    {
        pCtx += ",";
    }
}

inline void encode_per(const RpcRequest& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    encode_per(pIe.uuid, pCtx);
    encode_per(pIe.param, pCtx);
}

inline void decode_per(RpcRequest& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    decode_per(pIe.uuid, pCtx);
    decode_per(pIe.param, pCtx);
}

inline void str(const char* pName, const RpcRequest& pIe, std::string& pCtx, bool pIsLast)
{
    using namespace cum;
    if (!pName)
    {
        pCtx = pCtx + "{";
    }
    else
    {
        pCtx = pCtx + "\"" + pName + "\":{";
    }
    size_t nOptional = 0;
    size_t nMandatory = 2;
    str("uuid", pIe.uuid, pCtx, !(--nMandatory+nOptional));
    str("param", pIe.param, pCtx, !(--nMandatory+nOptional));
    pCtx = pCtx + "}";
    if (!pIsLast)
    {
        pCtx += ",";
    }
}

inline void encode_per(const RpcAccept& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    encode_per(pIe.value, pCtx);
}

inline void decode_per(RpcAccept& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    decode_per(pIe.value, pCtx);
}

inline void str(const char* pName, const RpcAccept& pIe, std::string& pCtx, bool pIsLast)
{
    using namespace cum;
    if (!pName)
    {
        pCtx = pCtx + "{";
    }
    else
    {
        pCtx = pCtx + "\"" + pName + "\":{";
    }
    size_t nOptional = 0;
    size_t nMandatory = 1;
    str("value", pIe.value, pCtx, !(--nMandatory+nOptional));
    pCtx = pCtx + "}";
    if (!pIsLast)
    {
        pCtx += ",";
    }
}

inline void encode_per(const RpcReject& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    encode_per(pIe.cause, pCtx);
}

inline void decode_per(RpcReject& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    decode_per(pIe.cause, pCtx);
}

inline void str(const char* pName, const RpcReject& pIe, std::string& pCtx, bool pIsLast)
{
    using namespace cum;
    if (!pName)
    {
        pCtx = pCtx + "{";
    }
    else
    {
        pCtx = pCtx + "\"" + pName + "\":{";
    }
    size_t nOptional = 0;
    size_t nMandatory = 1;
    str("cause", pIe.cause, pCtx, !(--nMandatory+nOptional));
    pCtx = pCtx + "}";
    if (!pIsLast)
    {
        pCtx += ",";
    }
}

inline void encode_per(const PropertyTreeMessages& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    using TypeIndex = uint8_t;
    TypeIndex type = pIe.index();
    encode_per(type, pCtx);
    if (0 == type)
    {
        encode_per(std::get<0>(pIe), pCtx);
    }
    else if (1 == type)
    {
        encode_per(std::get<1>(pIe), pCtx);
    }
    else if (2 == type)
    {
        encode_per(std::get<2>(pIe), pCtx);
    }
    else if (3 == type)
    {
        encode_per(std::get<3>(pIe), pCtx);
    }
    else if (4 == type)
    {
        encode_per(std::get<4>(pIe), pCtx);
    }
    else if (5 == type)
    {
        encode_per(std::get<5>(pIe), pCtx);
    }
    else if (6 == type)
    {
        encode_per(std::get<6>(pIe), pCtx);
    }
    else if (7 == type)
    {
        encode_per(std::get<7>(pIe), pCtx);
    }
    else if (8 == type)
    {
        encode_per(std::get<8>(pIe), pCtx);
    }
    else if (9 == type)
    {
        encode_per(std::get<9>(pIe), pCtx);
    }
    else if (10 == type)
    {
        encode_per(std::get<10>(pIe), pCtx);
    }
    else if (11 == type)
    {
        encode_per(std::get<11>(pIe), pCtx);
    }
    else if (12 == type)
    {
        encode_per(std::get<12>(pIe), pCtx);
    }
    else if (13 == type)
    {
        encode_per(std::get<13>(pIe), pCtx);
    }
    else if (14 == type)
    {
        encode_per(std::get<14>(pIe), pCtx);
    }
    else if (15 == type)
    {
        encode_per(std::get<15>(pIe), pCtx);
    }
    else if (16 == type)
    {
        encode_per(std::get<16>(pIe), pCtx);
    }
    else if (17 == type)
    {
        encode_per(std::get<17>(pIe), pCtx);
    }
    else if (18 == type)
    {
        encode_per(std::get<18>(pIe), pCtx);
    }
    else if (19 == type)
    {
        encode_per(std::get<19>(pIe), pCtx);
    }
    else if (20 == type)
    {
        encode_per(std::get<20>(pIe), pCtx);
    }
    else if (21 == type)
    {
        encode_per(std::get<21>(pIe), pCtx);
    }
    else if (22 == type)
    {
        encode_per(std::get<22>(pIe), pCtx);
    }
}

inline void decode_per(PropertyTreeMessages& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    using TypeIndex = uint8_t;
    TypeIndex type;
    decode_per(type, pCtx);
    if (0 == type)
    {
        pIe = SigninRequest();
        decode_per(std::get<0>(pIe), pCtx);
    }
    else if (1 == type)
    {
        pIe = SigninAccept();
        decode_per(std::get<1>(pIe), pCtx);
    }
    else if (2 == type)
    {
        pIe = CreateRequest();
        decode_per(std::get<2>(pIe), pCtx);
    }
    else if (3 == type)
    {
        pIe = CreateAccept();
        decode_per(std::get<3>(pIe), pCtx);
    }
    else if (4 == type)
    {
        pIe = CreateReject();
        decode_per(std::get<4>(pIe), pCtx);
    }
    else if (5 == type)
    {
        pIe = GetRequest();
        decode_per(std::get<5>(pIe), pCtx);
    }
    else if (6 == type)
    {
        pIe = GetAccept();
        decode_per(std::get<6>(pIe), pCtx);
    }
    else if (7 == type)
    {
        pIe = GetReject();
        decode_per(std::get<7>(pIe), pCtx);
    }
    else if (8 == type)
    {
        pIe = TreeInfoRequest();
        decode_per(std::get<8>(pIe), pCtx);
    }
    else if (9 == type)
    {
        pIe = TreeInfoResponse();
        decode_per(std::get<9>(pIe), pCtx);
    }
    else if (10 == type)
    {
        pIe = TreeInfoErrorResponse();
        decode_per(std::get<10>(pIe), pCtx);
    }
    else if (11 == type)
    {
        pIe = TreeUpdateNotification();
        decode_per(std::get<11>(pIe), pCtx);
    }
    else if (12 == type)
    {
        pIe = DeleteRequest();
        decode_per(std::get<12>(pIe), pCtx);
    }
    else if (13 == type)
    {
        pIe = DeleteResponse();
        decode_per(std::get<13>(pIe), pCtx);
    }
    else if (14 == type)
    {
        pIe = SetValueIndication();
        decode_per(std::get<14>(pIe), pCtx);
    }
    else if (15 == type)
    {
        pIe = SubscribeRequest();
        decode_per(std::get<15>(pIe), pCtx);
    }
    else if (16 == type)
    {
        pIe = SubscribeResponse();
        decode_per(std::get<16>(pIe), pCtx);
    }
    else if (17 == type)
    {
        pIe = UnsubscribeRequest();
        decode_per(std::get<17>(pIe), pCtx);
    }
    else if (18 == type)
    {
        pIe = UnsubscribeResponse();
        decode_per(std::get<18>(pIe), pCtx);
    }
    else if (19 == type)
    {
        pIe = UpdateNotification();
        decode_per(std::get<19>(pIe), pCtx);
    }
    else if (20 == type)
    {
        pIe = RpcRequest();
        decode_per(std::get<20>(pIe), pCtx);
    }
    else if (21 == type)
    {
        pIe = RpcAccept();
        decode_per(std::get<21>(pIe), pCtx);
    }
    else if (22 == type)
    {
        pIe = RpcReject();
        decode_per(std::get<22>(pIe), pCtx);
    }
}

inline void str(const char* pName, const PropertyTreeMessages& pIe, std::string& pCtx, bool pIsLast)
{
    using namespace cum;
    using TypeIndex = uint8_t;
    TypeIndex type = pIe.index();
    if (0 == type)
    {
        if (pName)
            pCtx += std::string(pName) + ":{";
        else
            pCtx += "{";
        std::string name = "SigninRequest";
        str(name.c_str(), std::get<0>(pIe), pCtx, true);
        pCtx += "}";
    }
    else if (1 == type)
    {
        if (pName)
            pCtx += std::string(pName) + ":{";
        else
            pCtx += "{";
        std::string name = "SigninAccept";
        str(name.c_str(), std::get<1>(pIe), pCtx, true);
        pCtx += "}";
    }
    else if (2 == type)
    {
        if (pName)
            pCtx += std::string(pName) + ":{";
        else
            pCtx += "{";
        std::string name = "CreateRequest";
        str(name.c_str(), std::get<2>(pIe), pCtx, true);
        pCtx += "}";
    }
    else if (3 == type)
    {
        if (pName)
            pCtx += std::string(pName) + ":{";
        else
            pCtx += "{";
        std::string name = "CreateAccept";
        str(name.c_str(), std::get<3>(pIe), pCtx, true);
        pCtx += "}";
    }
    else if (4 == type)
    {
        if (pName)
            pCtx += std::string(pName) + ":{";
        else
            pCtx += "{";
        std::string name = "CreateReject";
        str(name.c_str(), std::get<4>(pIe), pCtx, true);
        pCtx += "}";
    }
    else if (5 == type)
    {
        if (pName)
            pCtx += std::string(pName) + ":{";
        else
            pCtx += "{";
        std::string name = "GetRequest";
        str(name.c_str(), std::get<5>(pIe), pCtx, true);
        pCtx += "}";
    }
    else if (6 == type)
    {
        if (pName)
            pCtx += std::string(pName) + ":{";
        else
            pCtx += "{";
        std::string name = "GetAccept";
        str(name.c_str(), std::get<6>(pIe), pCtx, true);
        pCtx += "}";
    }
    else if (7 == type)
    {
        if (pName)
            pCtx += std::string(pName) + ":{";
        else
            pCtx += "{";
        std::string name = "GetReject";
        str(name.c_str(), std::get<7>(pIe), pCtx, true);
        pCtx += "}";
    }
    else if (8 == type)
    {
        if (pName)
            pCtx += std::string(pName) + ":{";
        else
            pCtx += "{";
        std::string name = "TreeInfoRequest";
        str(name.c_str(), std::get<8>(pIe), pCtx, true);
        pCtx += "}";
    }
    else if (9 == type)
    {
        if (pName)
            pCtx += std::string(pName) + ":{";
        else
            pCtx += "{";
        std::string name = "TreeInfoResponse";
        str(name.c_str(), std::get<9>(pIe), pCtx, true);
        pCtx += "}";
    }
    else if (10 == type)
    {
        if (pName)
            pCtx += std::string(pName) + ":{";
        else
            pCtx += "{";
        std::string name = "TreeInfoErrorResponse";
        str(name.c_str(), std::get<10>(pIe), pCtx, true);
        pCtx += "}";
    }
    else if (11 == type)
    {
        if (pName)
            pCtx += std::string(pName) + ":{";
        else
            pCtx += "{";
        std::string name = "TreeUpdateNotification";
        str(name.c_str(), std::get<11>(pIe), pCtx, true);
        pCtx += "}";
    }
    else if (12 == type)
    {
        if (pName)
            pCtx += std::string(pName) + ":{";
        else
            pCtx += "{";
        std::string name = "DeleteRequest";
        str(name.c_str(), std::get<12>(pIe), pCtx, true);
        pCtx += "}";
    }
    else if (13 == type)
    {
        if (pName)
            pCtx += std::string(pName) + ":{";
        else
            pCtx += "{";
        std::string name = "DeleteResponse";
        str(name.c_str(), std::get<13>(pIe), pCtx, true);
        pCtx += "}";
    }
    else if (14 == type)
    {
        if (pName)
            pCtx += std::string(pName) + ":{";
        else
            pCtx += "{";
        std::string name = "SetValueIndication";
        str(name.c_str(), std::get<14>(pIe), pCtx, true);
        pCtx += "}";
    }
    else if (15 == type)
    {
        if (pName)
            pCtx += std::string(pName) + ":{";
        else
            pCtx += "{";
        std::string name = "SubscribeRequest";
        str(name.c_str(), std::get<15>(pIe), pCtx, true);
        pCtx += "}";
    }
    else if (16 == type)
    {
        if (pName)
            pCtx += std::string(pName) + ":{";
        else
            pCtx += "{";
        std::string name = "SubscribeResponse";
        str(name.c_str(), std::get<16>(pIe), pCtx, true);
        pCtx += "}";
    }
    else if (17 == type)
    {
        if (pName)
            pCtx += std::string(pName) + ":{";
        else
            pCtx += "{";
        std::string name = "UnsubscribeRequest";
        str(name.c_str(), std::get<17>(pIe), pCtx, true);
        pCtx += "}";
    }
    else if (18 == type)
    {
        if (pName)
            pCtx += std::string(pName) + ":{";
        else
            pCtx += "{";
        std::string name = "UnsubscribeResponse";
        str(name.c_str(), std::get<18>(pIe), pCtx, true);
        pCtx += "}";
    }
    else if (19 == type)
    {
        if (pName)
            pCtx += std::string(pName) + ":{";
        else
            pCtx += "{";
        std::string name = "UpdateNotification";
        str(name.c_str(), std::get<19>(pIe), pCtx, true);
        pCtx += "}";
    }
    else if (20 == type)
    {
        if (pName)
            pCtx += std::string(pName) + ":{";
        else
            pCtx += "{";
        std::string name = "RpcRequest";
        str(name.c_str(), std::get<20>(pIe), pCtx, true);
        pCtx += "}";
    }
    else if (21 == type)
    {
        if (pName)
            pCtx += std::string(pName) + ":{";
        else
            pCtx += "{";
        std::string name = "RpcAccept";
        str(name.c_str(), std::get<21>(pIe), pCtx, true);
        pCtx += "}";
    }
    else if (22 == type)
    {
        if (pName)
            pCtx += std::string(pName) + ":{";
        else
            pCtx += "{";
        std::string name = "RpcReject";
        str(name.c_str(), std::get<22>(pIe), pCtx, true);
        pCtx += "}";
    }
    if (!pIsLast)
    {
        pCtx += ",";
    }
}

inline void encode_per(const PropertyTreeMessage& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    encode_per(pIe.transactionId, pCtx);
    encode_per(pIe.message, pCtx);
}

inline void decode_per(PropertyTreeMessage& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    decode_per(pIe.transactionId, pCtx);
    decode_per(pIe.message, pCtx);
}

inline void str(const char* pName, const PropertyTreeMessage& pIe, std::string& pCtx, bool pIsLast)
{
    using namespace cum;
    if (!pName)
    {
        pCtx = pCtx + "{";
    }
    else
    {
        pCtx = pCtx + "\"" + pName + "\":{";
    }
    size_t nOptional = 0;
    size_t nMandatory = 2;
    str("transactionId", pIe.transactionId, pCtx, !(--nMandatory+nOptional));
    str("message", pIe.message, pCtx, !(--nMandatory+nOptional));
    pCtx = pCtx + "}";
    if (!pIsLast)
    {
        pCtx += ",";
    }
}

inline void encode_per(const PropertyTreeProtocol& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    using TypeIndex = uint8_t;
    TypeIndex type = pIe.index();
    encode_per(type, pCtx);
    if (0 == type)
    {
        encode_per(std::get<0>(pIe), pCtx);
    }
    else if (1 == type)
    {
        encode_per(std::get<1>(pIe), pCtx);
    }
}

inline void decode_per(PropertyTreeProtocol& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    using TypeIndex = uint8_t;
    TypeIndex type;
    decode_per(type, pCtx);
    if (0 == type)
    {
        pIe = PropertyTreeMessage();
        decode_per(std::get<0>(pIe), pCtx);
    }
    else if (1 == type)
    {
        pIe = PropertyTreeMessageArray();
        decode_per(std::get<1>(pIe), pCtx);
    }
}

inline void str(const char* pName, const PropertyTreeProtocol& pIe, std::string& pCtx, bool pIsLast)
{
    using namespace cum;
    using TypeIndex = uint8_t;
    TypeIndex type = pIe.index();
    if (0 == type)
    {
        if (pName)
            pCtx += std::string(pName) + ":{";
        else
            pCtx += "{";
        std::string name = "PropertyTreeMessage";
        str(name.c_str(), std::get<0>(pIe), pCtx, true);
        pCtx += "}";
    }
    else if (1 == type)
    {
        if (pName)
            pCtx += std::string(pName) + ":{";
        else
            pCtx += "{";
        std::string name = "PropertyTreeMessageArray";
        str(name.c_str(), std::get<1>(pIe), pCtx, true);
        pCtx += "}";
    }
    if (!pIsLast)
    {
        pCtx += ",";
    }
}

#endif //__CUM_MSG_HPP__
