// Type:  ('buffer', {'type': 'byte'})
// Type:  ('buffer', {'dynamic_array': '256'})
// Type:  ('u64_list', {'type': 'u64'})
// Type:  ('u64_list', {'dynamic_array': '256'})
// Type:  ('string_list', {'type': 'string'})
// Type:  ('string_list', {'dynamic_array': '256'})
// Constant:  ('NONTRANSACTIONAL', '0xFF')
// Enumeration:  ('EStatus', ('OK', None))
// Enumeration:  ('EStatus', ('INVALID_KEY', None))
// Enumeration:  ('EStatus', ('INVALID_SIZE', None))
// Sequence:  acknowledge ('u16', 'transaction_id')
// Sequence:  acknowledge ('u16', 'status')
// Sequence:  value ('u32', 'key')
// Sequence:  value ('buffer', 'value')
// Sequence:  set_value ('u16', 'transaction_id')
// Sequence:  set_value ('value', 'data')
// Sequence:  get_value_request ('u16', 'transaction_id')
// Sequence:  get_value_request ('u32', 'key')
// Sequence:  get_value_response ('u16', 'transaction_id')
// Sequence:  get_value_response ('value', 'data')
// Sequence:  get_value_response ('u64', 'sequence_number')
// Sequence:  subscribe ('u16', 'transaction_id')
// Sequence:  subscribe ('u32', 'key')
// Sequence:  unsubscribe ('u16', 'transaction_id')
// Sequence:  unsubscribe ('u32', 'key')
// Sequence:  update ('value', 'data')
// Sequence:  update ('u64', 'sequence_number')
// Choice:  ('protocol_value_server', 'set_value')
// Choice:  ('protocol_value_server', 'get_value_request')
// Choice:  ('protocol_value_server', 'subscribe')
// Choice:  ('protocol_value_server', 'unsubscribe')
// Choice:  ('protocol_value_client', 'acknowledge')
// Choice:  ('protocol_value_client', 'get_value_response')
// Choice:  ('protocol_value_client', 'update')
// Sequence:  node ('u32', 'key')
// Sequence:  node ('u32', 'parent')
// Sequence:  node ('string', 'name')
// Type:  ('node_list', {'type': 'node'})
// Type:  ('node_list', {'dynamic_array': '4294967296'})
// Type:  ('node_optional', {'type': 'node'})
// Type:  ('node_optional', {'optional': None})
// Type:  ('node_attach_list', {'type': 'node_list'})
// Type:  ('node_detach_list', {'type': 'u64_list'})
// Sequence:  list_request ('u16', 'transaction_id')
// Sequence:  list_request ('u32', 'key')
// Sequence:  list_response ('u16', 'transaction_id')
// Sequence:  list_response ('node_list', 'nodes')
// Sequence:  attach ('u16', 'transaction_id')
// Sequence:  attach ('u64', 'parent')
// Sequence:  attach ('u32', 'key')
// Sequence:  attach ('string', 'name')
// Sequence:  detach ('u16', 'transaction_id')
// Sequence:  detach ('u32', 'key')
// Sequence:  node_subscribe ('u16', 'transaction_id')
// Sequence:  node_subscribe ('u32', 'key')
// Sequence:  node_unsubscribe ('u16', 'transaction_id')
// Sequence:  node_unsubscribe ('u32', 'key')
// Sequence:  node_update_attach ('node', 'to_attach')
// Sequence:  node_update_detach ('u32', 'to_detach')
// Choice:  ('protocol_node_server', 'list_request')
// Choice:  ('protocol_node_server', 'attach')
// Choice:  ('protocol_node_server', 'detach')
// Choice:  ('protocol_node_server', 'node_subscribe')
// Choice:  ('protocol_node_server', 'node_unsubscribe')
// Choice:  ('protocol_node_client', 'acknowledge')
// Choice:  ('protocol_node_client', 'list_response')
// Choice:  ('protocol_node_client', 'node_update_attach')
// Choice:  ('protocol_node_client', 'node_update_detach')
// Generating for C++
#ifndef __CUM_MSG_HPP__
#define __CUM_MSG_HPP__
#include "cum/cum.hpp"

namespace cum
{

/***********************************************
/
/            Message Definitions
/
************************************************/

using buffer = cum::vector<byte, 256>;
using u64_list = cum::vector<u64, 256>;
using string_list = cum::vector<string, 256>;
constexpr auto NONTRANSACTIONAL = 0xFF;
enum EStatus
{
    OK,
    INVALID_KEY,
    INVALID_SIZE
};

struct acknowledge
{
    u16 transaction_id;
    u16 status;
};

struct value
{
    u32 key;
    buffer value;
};

struct set_value
{
    u16 transaction_id;
    value data;
};

struct get_value_request
{
    u16 transaction_id;
    u32 key;
};

struct get_value_response
{
    u16 transaction_id;
    value data;
    u64 sequence_number;
};

struct subscribe
{
    u16 transaction_id;
    u32 key;
};

struct unsubscribe
{
    u16 transaction_id;
    u32 key;
};

struct update
{
    value data;
    u64 sequence_number;
};

using protocol_value_server = std::variant<set_value,get_value_request,subscribe,unsubscribe>;
using protocol_value_client = std::variant<acknowledge,get_value_response,update>;
struct node
{
    u32 key;
    u32 parent;
    string name;
};

using node_list = cum::vector<node, 4294967296>;
using node_optional = std::optional<node>;
using node_attach_list = node_list;
using node_detach_list = u64_list;
struct list_request
{
    u16 transaction_id;
    u32 key;
};

struct list_response
{
    u16 transaction_id;
    node_list nodes;
};

struct attach
{
    u16 transaction_id;
    u64 parent;
    u32 key;
    string name;
};

struct detach
{
    u16 transaction_id;
    u32 key;
};

struct node_subscribe
{
    u16 transaction_id;
    u32 key;
};

struct node_unsubscribe
{
    u16 transaction_id;
    u32 key;
};

struct node_update_attach
{
    node to_attach;
};

struct node_update_detach
{
    u32 to_detach;
};

using protocol_node_server = std::variant<list_request,attach,detach,node_subscribe,node_unsubscribe>;
using protocol_node_client = std::variant<acknowledge,list_response,node_update_attach,node_update_detach>;
/***********************************************
/
/            Codec Definitions
/
************************************************/

inline void str(const char* pName, const EStatus& pIe, std::string& pCtx, bool pIsLast)
{
    using namespace cum;
    if (pName)
    {
        pCtx = pCtx + "\"" + pName + "\":";
    }
    if (EStatus::OK == pIe) pCtx += "\"OK\"";
    if (EStatus::INVALID_KEY == pIe) pCtx += "\"INVALID_KEY\"";
    if (EStatus::INVALID_SIZE == pIe) pCtx += "\"INVALID_SIZE\"";
    pCtx = pCtx + "}";
    if (!pIsLast)
    {
        pCtx += ",";
    }
}

inline void encode_per(const acknowledge& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    encode_per(pIe.transaction_id, pCtx);
    encode_per(pIe.status, pCtx);
}

inline void decode_per(acknowledge& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    decode_per(pIe.transaction_id, pCtx);
    decode_per(pIe.status, pCtx);
}

inline void str(const char* pName, const acknowledge& pIe, std::string& pCtx, bool pIsLast)
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
    str("transaction_id", pIe.transaction_id, pCtx, !(--nMandatory+nOptional));
    str("status", pIe.status, pCtx, !(--nMandatory+nOptional));
    pCtx = pCtx + "}";
    if (!pIsLast)
    {
        pCtx += ",";
    }
}

inline void encode_per(const value& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    encode_per(pIe.key, pCtx);
    encode_per(pIe.value, pCtx);
}

inline void decode_per(value& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    decode_per(pIe.key, pCtx);
    decode_per(pIe.value, pCtx);
}

inline void str(const char* pName, const value& pIe, std::string& pCtx, bool pIsLast)
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
    str("key", pIe.key, pCtx, !(--nMandatory+nOptional));
    str("value", pIe.value, pCtx, !(--nMandatory+nOptional));
    pCtx = pCtx + "}";
    if (!pIsLast)
    {
        pCtx += ",";
    }
}

inline void encode_per(const set_value& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    encode_per(pIe.transaction_id, pCtx);
    encode_per(pIe.data, pCtx);
}

inline void decode_per(set_value& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    decode_per(pIe.transaction_id, pCtx);
    decode_per(pIe.data, pCtx);
}

inline void str(const char* pName, const set_value& pIe, std::string& pCtx, bool pIsLast)
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
    str("transaction_id", pIe.transaction_id, pCtx, !(--nMandatory+nOptional));
    str("data", pIe.data, pCtx, !(--nMandatory+nOptional));
    pCtx = pCtx + "}";
    if (!pIsLast)
    {
        pCtx += ",";
    }
}

inline void encode_per(const get_value_request& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    encode_per(pIe.transaction_id, pCtx);
    encode_per(pIe.key, pCtx);
}

inline void decode_per(get_value_request& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    decode_per(pIe.transaction_id, pCtx);
    decode_per(pIe.key, pCtx);
}

inline void str(const char* pName, const get_value_request& pIe, std::string& pCtx, bool pIsLast)
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
    str("transaction_id", pIe.transaction_id, pCtx, !(--nMandatory+nOptional));
    str("key", pIe.key, pCtx, !(--nMandatory+nOptional));
    pCtx = pCtx + "}";
    if (!pIsLast)
    {
        pCtx += ",";
    }
}

inline void encode_per(const get_value_response& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    encode_per(pIe.transaction_id, pCtx);
    encode_per(pIe.data, pCtx);
    encode_per(pIe.sequence_number, pCtx);
}

inline void decode_per(get_value_response& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    decode_per(pIe.transaction_id, pCtx);
    decode_per(pIe.data, pCtx);
    decode_per(pIe.sequence_number, pCtx);
}

inline void str(const char* pName, const get_value_response& pIe, std::string& pCtx, bool pIsLast)
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
    str("transaction_id", pIe.transaction_id, pCtx, !(--nMandatory+nOptional));
    str("data", pIe.data, pCtx, !(--nMandatory+nOptional));
    str("sequence_number", pIe.sequence_number, pCtx, !(--nMandatory+nOptional));
    pCtx = pCtx + "}";
    if (!pIsLast)
    {
        pCtx += ",";
    }
}

inline void encode_per(const subscribe& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    encode_per(pIe.transaction_id, pCtx);
    encode_per(pIe.key, pCtx);
}

inline void decode_per(subscribe& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    decode_per(pIe.transaction_id, pCtx);
    decode_per(pIe.key, pCtx);
}

inline void str(const char* pName, const subscribe& pIe, std::string& pCtx, bool pIsLast)
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
    str("transaction_id", pIe.transaction_id, pCtx, !(--nMandatory+nOptional));
    str("key", pIe.key, pCtx, !(--nMandatory+nOptional));
    pCtx = pCtx + "}";
    if (!pIsLast)
    {
        pCtx += ",";
    }
}

inline void encode_per(const unsubscribe& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    encode_per(pIe.transaction_id, pCtx);
    encode_per(pIe.key, pCtx);
}

inline void decode_per(unsubscribe& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    decode_per(pIe.transaction_id, pCtx);
    decode_per(pIe.key, pCtx);
}

inline void str(const char* pName, const unsubscribe& pIe, std::string& pCtx, bool pIsLast)
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
    str("transaction_id", pIe.transaction_id, pCtx, !(--nMandatory+nOptional));
    str("key", pIe.key, pCtx, !(--nMandatory+nOptional));
    pCtx = pCtx + "}";
    if (!pIsLast)
    {
        pCtx += ",";
    }
}

inline void encode_per(const update& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    encode_per(pIe.data, pCtx);
    encode_per(pIe.sequence_number, pCtx);
}

inline void decode_per(update& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    decode_per(pIe.data, pCtx);
    decode_per(pIe.sequence_number, pCtx);
}

inline void str(const char* pName, const update& pIe, std::string& pCtx, bool pIsLast)
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
    str("data", pIe.data, pCtx, !(--nMandatory+nOptional));
    str("sequence_number", pIe.sequence_number, pCtx, !(--nMandatory+nOptional));
    pCtx = pCtx + "}";
    if (!pIsLast)
    {
        pCtx += ",";
    }
}

inline void encode_per(const protocol_value_server& pIe, cum::per_codec_ctx& pCtx)
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
}

inline void decode_per(protocol_value_server& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    using TypeIndex = uint8_t;
    TypeIndex type;
    decode_per(type, pCtx);
    if (0 == type)
    {
        pIe = set_value();
        decode_per(std::get<0>(pIe), pCtx);
    }
    else if (1 == type)
    {
        pIe = get_value_request();
        decode_per(std::get<1>(pIe), pCtx);
    }
    else if (2 == type)
    {
        pIe = subscribe();
        decode_per(std::get<2>(pIe), pCtx);
    }
    else if (3 == type)
    {
        pIe = unsubscribe();
        decode_per(std::get<3>(pIe), pCtx);
    }
}

inline void str(const char* pName, const protocol_value_server& pIe, std::string& pCtx, bool pIsLast)
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
        std::string name = "set_value";
        str(name.c_str(), std::get<0>(pIe), pCtx, true);
        pCtx += "}";
    }
    else if (1 == type)
    {
        if (pName)
            pCtx += std::string(pName) + ":{";
        else
            pCtx += "{";
        std::string name = "get_value_request";
        str(name.c_str(), std::get<1>(pIe), pCtx, true);
        pCtx += "}";
    }
    else if (2 == type)
    {
        if (pName)
            pCtx += std::string(pName) + ":{";
        else
            pCtx += "{";
        std::string name = "subscribe";
        str(name.c_str(), std::get<2>(pIe), pCtx, true);
        pCtx += "}";
    }
    else if (3 == type)
    {
        if (pName)
            pCtx += std::string(pName) + ":{";
        else
            pCtx += "{";
        std::string name = "unsubscribe";
        str(name.c_str(), std::get<3>(pIe), pCtx, true);
        pCtx += "}";
    }
    if (!pIsLast)
    {
        pCtx += ",";
    }
}

inline void encode_per(const protocol_value_client& pIe, cum::per_codec_ctx& pCtx)
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
}

inline void decode_per(protocol_value_client& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    using TypeIndex = uint8_t;
    TypeIndex type;
    decode_per(type, pCtx);
    if (0 == type)
    {
        pIe = acknowledge();
        decode_per(std::get<0>(pIe), pCtx);
    }
    else if (1 == type)
    {
        pIe = get_value_response();
        decode_per(std::get<1>(pIe), pCtx);
    }
    else if (2 == type)
    {
        pIe = update();
        decode_per(std::get<2>(pIe), pCtx);
    }
}

inline void str(const char* pName, const protocol_value_client& pIe, std::string& pCtx, bool pIsLast)
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
        std::string name = "acknowledge";
        str(name.c_str(), std::get<0>(pIe), pCtx, true);
        pCtx += "}";
    }
    else if (1 == type)
    {
        if (pName)
            pCtx += std::string(pName) + ":{";
        else
            pCtx += "{";
        std::string name = "get_value_response";
        str(name.c_str(), std::get<1>(pIe), pCtx, true);
        pCtx += "}";
    }
    else if (2 == type)
    {
        if (pName)
            pCtx += std::string(pName) + ":{";
        else
            pCtx += "{";
        std::string name = "update";
        str(name.c_str(), std::get<2>(pIe), pCtx, true);
        pCtx += "}";
    }
    if (!pIsLast)
    {
        pCtx += ",";
    }
}

inline void encode_per(const node& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    encode_per(pIe.key, pCtx);
    encode_per(pIe.parent, pCtx);
    encode_per(pIe.name, pCtx);
}

inline void decode_per(node& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    decode_per(pIe.key, pCtx);
    decode_per(pIe.parent, pCtx);
    decode_per(pIe.name, pCtx);
}

inline void str(const char* pName, const node& pIe, std::string& pCtx, bool pIsLast)
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
    str("key", pIe.key, pCtx, !(--nMandatory+nOptional));
    str("parent", pIe.parent, pCtx, !(--nMandatory+nOptional));
    str("name", pIe.name, pCtx, !(--nMandatory+nOptional));
    pCtx = pCtx + "}";
    if (!pIsLast)
    {
        pCtx += ",";
    }
}

inline void encode_per(const list_request& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    encode_per(pIe.transaction_id, pCtx);
    encode_per(pIe.key, pCtx);
}

inline void decode_per(list_request& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    decode_per(pIe.transaction_id, pCtx);
    decode_per(pIe.key, pCtx);
}

inline void str(const char* pName, const list_request& pIe, std::string& pCtx, bool pIsLast)
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
    str("transaction_id", pIe.transaction_id, pCtx, !(--nMandatory+nOptional));
    str("key", pIe.key, pCtx, !(--nMandatory+nOptional));
    pCtx = pCtx + "}";
    if (!pIsLast)
    {
        pCtx += ",";
    }
}

inline void encode_per(const list_response& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    encode_per(pIe.transaction_id, pCtx);
    encode_per(pIe.nodes, pCtx);
}

inline void decode_per(list_response& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    decode_per(pIe.transaction_id, pCtx);
    decode_per(pIe.nodes, pCtx);
}

inline void str(const char* pName, const list_response& pIe, std::string& pCtx, bool pIsLast)
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
    str("transaction_id", pIe.transaction_id, pCtx, !(--nMandatory+nOptional));
    str("nodes", pIe.nodes, pCtx, !(--nMandatory+nOptional));
    pCtx = pCtx + "}";
    if (!pIsLast)
    {
        pCtx += ",";
    }
}

inline void encode_per(const attach& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    encode_per(pIe.transaction_id, pCtx);
    encode_per(pIe.parent, pCtx);
    encode_per(pIe.key, pCtx);
    encode_per(pIe.name, pCtx);
}

inline void decode_per(attach& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    decode_per(pIe.transaction_id, pCtx);
    decode_per(pIe.parent, pCtx);
    decode_per(pIe.key, pCtx);
    decode_per(pIe.name, pCtx);
}

inline void str(const char* pName, const attach& pIe, std::string& pCtx, bool pIsLast)
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
    size_t nMandatory = 4;
    str("transaction_id", pIe.transaction_id, pCtx, !(--nMandatory+nOptional));
    str("parent", pIe.parent, pCtx, !(--nMandatory+nOptional));
    str("key", pIe.key, pCtx, !(--nMandatory+nOptional));
    str("name", pIe.name, pCtx, !(--nMandatory+nOptional));
    pCtx = pCtx + "}";
    if (!pIsLast)
    {
        pCtx += ",";
    }
}

inline void encode_per(const detach& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    encode_per(pIe.transaction_id, pCtx);
    encode_per(pIe.key, pCtx);
}

inline void decode_per(detach& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    decode_per(pIe.transaction_id, pCtx);
    decode_per(pIe.key, pCtx);
}

inline void str(const char* pName, const detach& pIe, std::string& pCtx, bool pIsLast)
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
    str("transaction_id", pIe.transaction_id, pCtx, !(--nMandatory+nOptional));
    str("key", pIe.key, pCtx, !(--nMandatory+nOptional));
    pCtx = pCtx + "}";
    if (!pIsLast)
    {
        pCtx += ",";
    }
}

inline void encode_per(const node_subscribe& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    encode_per(pIe.transaction_id, pCtx);
    encode_per(pIe.key, pCtx);
}

inline void decode_per(node_subscribe& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    decode_per(pIe.transaction_id, pCtx);
    decode_per(pIe.key, pCtx);
}

inline void str(const char* pName, const node_subscribe& pIe, std::string& pCtx, bool pIsLast)
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
    str("transaction_id", pIe.transaction_id, pCtx, !(--nMandatory+nOptional));
    str("key", pIe.key, pCtx, !(--nMandatory+nOptional));
    pCtx = pCtx + "}";
    if (!pIsLast)
    {
        pCtx += ",";
    }
}

inline void encode_per(const node_unsubscribe& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    encode_per(pIe.transaction_id, pCtx);
    encode_per(pIe.key, pCtx);
}

inline void decode_per(node_unsubscribe& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    decode_per(pIe.transaction_id, pCtx);
    decode_per(pIe.key, pCtx);
}

inline void str(const char* pName, const node_unsubscribe& pIe, std::string& pCtx, bool pIsLast)
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
    str("transaction_id", pIe.transaction_id, pCtx, !(--nMandatory+nOptional));
    str("key", pIe.key, pCtx, !(--nMandatory+nOptional));
    pCtx = pCtx + "}";
    if (!pIsLast)
    {
        pCtx += ",";
    }
}

inline void encode_per(const node_update_attach& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    encode_per(pIe.to_attach, pCtx);
}

inline void decode_per(node_update_attach& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    decode_per(pIe.to_attach, pCtx);
}

inline void str(const char* pName, const node_update_attach& pIe, std::string& pCtx, bool pIsLast)
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
    str("to_attach", pIe.to_attach, pCtx, !(--nMandatory+nOptional));
    pCtx = pCtx + "}";
    if (!pIsLast)
    {
        pCtx += ",";
    }
}

inline void encode_per(const node_update_detach& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    encode_per(pIe.to_detach, pCtx);
}

inline void decode_per(node_update_detach& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    decode_per(pIe.to_detach, pCtx);
}

inline void str(const char* pName, const node_update_detach& pIe, std::string& pCtx, bool pIsLast)
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
    str("to_detach", pIe.to_detach, pCtx, !(--nMandatory+nOptional));
    pCtx = pCtx + "}";
    if (!pIsLast)
    {
        pCtx += ",";
    }
}

inline void encode_per(const protocol_node_server& pIe, cum::per_codec_ctx& pCtx)
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
}

inline void decode_per(protocol_node_server& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    using TypeIndex = uint8_t;
    TypeIndex type;
    decode_per(type, pCtx);
    if (0 == type)
    {
        pIe = list_request();
        decode_per(std::get<0>(pIe), pCtx);
    }
    else if (1 == type)
    {
        pIe = attach();
        decode_per(std::get<1>(pIe), pCtx);
    }
    else if (2 == type)
    {
        pIe = detach();
        decode_per(std::get<2>(pIe), pCtx);
    }
    else if (3 == type)
    {
        pIe = node_subscribe();
        decode_per(std::get<3>(pIe), pCtx);
    }
    else if (4 == type)
    {
        pIe = node_unsubscribe();
        decode_per(std::get<4>(pIe), pCtx);
    }
}

inline void str(const char* pName, const protocol_node_server& pIe, std::string& pCtx, bool pIsLast)
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
        std::string name = "list_request";
        str(name.c_str(), std::get<0>(pIe), pCtx, true);
        pCtx += "}";
    }
    else if (1 == type)
    {
        if (pName)
            pCtx += std::string(pName) + ":{";
        else
            pCtx += "{";
        std::string name = "attach";
        str(name.c_str(), std::get<1>(pIe), pCtx, true);
        pCtx += "}";
    }
    else if (2 == type)
    {
        if (pName)
            pCtx += std::string(pName) + ":{";
        else
            pCtx += "{";
        std::string name = "detach";
        str(name.c_str(), std::get<2>(pIe), pCtx, true);
        pCtx += "}";
    }
    else if (3 == type)
    {
        if (pName)
            pCtx += std::string(pName) + ":{";
        else
            pCtx += "{";
        std::string name = "node_subscribe";
        str(name.c_str(), std::get<3>(pIe), pCtx, true);
        pCtx += "}";
    }
    else if (4 == type)
    {
        if (pName)
            pCtx += std::string(pName) + ":{";
        else
            pCtx += "{";
        std::string name = "node_unsubscribe";
        str(name.c_str(), std::get<4>(pIe), pCtx, true);
        pCtx += "}";
    }
    if (!pIsLast)
    {
        pCtx += ",";
    }
}

inline void encode_per(const protocol_node_client& pIe, cum::per_codec_ctx& pCtx)
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
}

inline void decode_per(protocol_node_client& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    using TypeIndex = uint8_t;
    TypeIndex type;
    decode_per(type, pCtx);
    if (0 == type)
    {
        pIe = acknowledge();
        decode_per(std::get<0>(pIe), pCtx);
    }
    else if (1 == type)
    {
        pIe = list_response();
        decode_per(std::get<1>(pIe), pCtx);
    }
    else if (2 == type)
    {
        pIe = node_update_attach();
        decode_per(std::get<2>(pIe), pCtx);
    }
    else if (3 == type)
    {
        pIe = node_update_detach();
        decode_per(std::get<3>(pIe), pCtx);
    }
}

inline void str(const char* pName, const protocol_node_client& pIe, std::string& pCtx, bool pIsLast)
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
        std::string name = "acknowledge";
        str(name.c_str(), std::get<0>(pIe), pCtx, true);
        pCtx += "}";
    }
    else if (1 == type)
    {
        if (pName)
            pCtx += std::string(pName) + ":{";
        else
            pCtx += "{";
        std::string name = "list_response";
        str(name.c_str(), std::get<1>(pIe), pCtx, true);
        pCtx += "}";
    }
    else if (2 == type)
    {
        if (pName)
            pCtx += std::string(pName) + ":{";
        else
            pCtx += "{";
        std::string name = "node_update_attach";
        str(name.c_str(), std::get<2>(pIe), pCtx, true);
        pCtx += "}";
    }
    else if (3 == type)
    {
        if (pName)
            pCtx += std::string(pName) + ":{";
        else
            pCtx += "{";
        std::string name = "node_update_detach";
        str(name.c_str(), std::get<3>(pIe), pCtx, true);
        pCtx += "}";
    }
    if (!pIsLast)
    {
        pCtx += ",";
    }
}

} // namespace cum
#endif //__CUM_MSG_HPP__
