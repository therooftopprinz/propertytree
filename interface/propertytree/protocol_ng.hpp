// Type:  ('buf', {'type': 'u8'})
// Type:  ('buf', {'dynamic_array': '256'})
// Type:  ('u64_list', {'type': 'u64'})
// Type:  ('u64_list', {'dynamic_array': '256'})
// Type:  ('string_list', {'type': 'string'})
// Type:  ('string_list', {'dynamic_array': '256'})
// Sequence:  value ('u64', 'id')
// Sequence:  value ('buf', 'data')
// Sequence:  subscribe ('u64', 'id')
// Sequence:  unsubscribe ('u64', 'id')
// Sequence:  update ('value', 'data')
// Sequence:  update ('u64', 'sn')
// Sequence:  node ('u64', 'id')
// Sequence:  node ('u64', 'parent')
// Sequence:  node ('string', 'name')
// Type:  ('node_list', {'type': 'node'})
// Type:  ('node_list', {'dynamic_array': '4294967296'})
// Type:  ('node_optional', {'type': 'node'})
// Type:  ('node_optional', {'optional': None})
// Type:  ('node_attach_list', {'type': 'node_list'})
// Type:  ('node_detach_list', {'type': 'u64_list'})
// Sequence:  list_request ('u16', 'transaction_id')
// Sequence:  list_request ('u64', 'id')
// Sequence:  list_response ('u16', 'transaction_id')
// Sequence:  list_response ('node_list', 'nodes')
// Sequence:  list_response ('u64', 'sequence_number')
// Sequence:  list_all_request ('u16', 'transaction_id')
// Sequence:  list_all_response ('u16', 'transaction_id')
// Sequence:  list_all_response ('node_list', 'nodes')
// Sequence:  list_all_response ('u64', 'sequence_number')
// Sequence:  resolve_path_request ('u16', 'transaction_id')
// Sequence:  resolve_path_request ('u64', 'origin')
// Sequence:  resolve_path_request ('string', 'path')
// Sequence:  resolve_path_response ('u16', 'transaction_id')
// Sequence:  resolve_path_response ('node_optional', 'node')
// Sequence:  resolve_path_response ('u64', 'sequence_number')
// Sequence:  attach ('u16', 'transaction_id')
// Sequence:  attach ('u64', 'parent')
// Sequence:  attach ('u64', 'id')
// Sequence:  attach ('string', 'name')
// Sequence:  detach ('u16', 'transaction_id')
// Sequence:  detach ('u64', 'id')
// Sequence:  node_subscribe ('u64', 'id')
// Sequence:  node_unsubscribe ('u64', 'id')
// Sequence:  node_update_attach ('node', 'to_attach')
// Sequence:  node_update_attach ('u64', 'sequence_number')
// Sequence:  node_update_detach ('u64', 'to_detach')
// Sequence:  node_update_detach ('u64', 'sequence_number')
// Choice:  ('protocol_value_server', 'subscribe')
// Choice:  ('protocol_value_server', 'unsubscribe')
// Choice:  ('protocol_value_client', 'update')
// Choice:  ('protocol_node_server', 'list_request')
// Choice:  ('protocol_node_server', 'list_all_request')
// Choice:  ('protocol_node_server', 'resolve_path_request')
// Choice:  ('protocol_node_server', 'attach')
// Choice:  ('protocol_node_server', 'detach')
// Choice:  ('protocol_node_server', 'node_subscribe')
// Choice:  ('protocol_node_server', 'node_unsubscribe')
// Choice:  ('protocol_node_client', 'list_response')
// Choice:  ('protocol_node_client', 'list_all_response')
// Choice:  ('protocol_node_client', 'resolve_path_response')
// Choice:  ('protocol_node_client', 'node_update_attach')
// Choice:  ('protocol_node_client', 'node_update_detach')
// Generating for C++
#ifndef __CUM_MSG_HPP__
#define __CUM_MSG_HPP__
#include "cum/cum.hpp"
#include <optional>

namespace cum
{

/***********************************************
/
/            Message Definitions
/
************************************************/

using buf = cum::vector<u8, 256>;
using u64_list = cum::vector<u64, 256>;
using string_list = cum::vector<string, 256>;
struct value
{
    u64 id;
    buf data;
};

struct subscribe
{
    u64 id;
};

struct unsubscribe
{
    u64 id;
};

struct update
{
    value data;
    u64 sn;
};

struct node
{
    u64 id;
    u64 parent;
    string name;
};

using node_list = cum::vector<node, 4294967296>;
using node_optional = std::optional<node>;
using node_attach_list = node_list;
using node_detach_list = u64_list;
struct list_request
{
    u16 transaction_id;
    u64 id;
};

struct list_response
{
    u16 transaction_id;
    node_list nodes;
    u64 sequence_number;
};

struct list_all_request
{
    u16 transaction_id;
};

struct list_all_response
{
    u16 transaction_id;
    node_list nodes;
    u64 sequence_number;
};

struct resolve_path_request
{
    u16 transaction_id;
    u64 origin;
    string path;
};

struct resolve_path_response
{
    u16 transaction_id;
    node_optional node;
    u64 sequence_number;
};

struct attach
{
    u16 transaction_id;
    u64 parent;
    u64 id;
    string name;
};

struct detach
{
    u16 transaction_id;
    u64 id;
};

struct node_subscribe
{
    u64 id;
};

struct node_unsubscribe
{
    u64 id;
};

struct node_update_attach
{
    node to_attach;
    u64 sequence_number;
};

struct node_update_detach
{
    u64 to_detach;
    u64 sequence_number;
};

using protocol_value_server = std::variant<subscribe,unsubscribe>;
using protocol_value_client = std::variant<update>;
using protocol_node_server = std::variant<list_request,list_all_request,resolve_path_request,attach,detach,node_subscribe,node_unsubscribe>;
using protocol_node_client = std::variant<list_response,list_all_response,resolve_path_response,node_update_attach,node_update_detach>;
/***********************************************
/
/            Codec Definitions
/
************************************************/

inline void encode_per(const value& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    encode_per(pIe.id, pCtx);
    encode_per(pIe.data, pCtx);
}

inline void decode_per(value& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    decode_per(pIe.id, pCtx);
    decode_per(pIe.data, pCtx);
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
    str("id", pIe.id, pCtx, !(--nMandatory+nOptional));
    str("data", pIe.data, pCtx, !(--nMandatory+nOptional));
    pCtx = pCtx + "}";
    if (!pIsLast)
    {
        pCtx += ",";
    }
}

inline void encode_per(const subscribe& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    encode_per(pIe.id, pCtx);
}

inline void decode_per(subscribe& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    decode_per(pIe.id, pCtx);
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
    size_t nMandatory = 1;
    str("id", pIe.id, pCtx, !(--nMandatory+nOptional));
    pCtx = pCtx + "}";
    if (!pIsLast)
    {
        pCtx += ",";
    }
}

inline void encode_per(const unsubscribe& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    encode_per(pIe.id, pCtx);
}

inline void decode_per(unsubscribe& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    decode_per(pIe.id, pCtx);
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
    size_t nMandatory = 1;
    str("id", pIe.id, pCtx, !(--nMandatory+nOptional));
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
    encode_per(pIe.sn, pCtx);
}

inline void decode_per(update& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    decode_per(pIe.data, pCtx);
    decode_per(pIe.sn, pCtx);
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
    str("sn", pIe.sn, pCtx, !(--nMandatory+nOptional));
    pCtx = pCtx + "}";
    if (!pIsLast)
    {
        pCtx += ",";
    }
}

inline void encode_per(const node& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    encode_per(pIe.id, pCtx);
    encode_per(pIe.parent, pCtx);
    encode_per(pIe.name, pCtx);
}

inline void decode_per(node& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    decode_per(pIe.id, pCtx);
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
    str("id", pIe.id, pCtx, !(--nMandatory+nOptional));
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
    encode_per(pIe.id, pCtx);
}

inline void decode_per(list_request& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    decode_per(pIe.transaction_id, pCtx);
    decode_per(pIe.id, pCtx);
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
    str("id", pIe.id, pCtx, !(--nMandatory+nOptional));
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
    encode_per(pIe.sequence_number, pCtx);
}

inline void decode_per(list_response& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    decode_per(pIe.transaction_id, pCtx);
    decode_per(pIe.nodes, pCtx);
    decode_per(pIe.sequence_number, pCtx);
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
    size_t nMandatory = 3;
    str("transaction_id", pIe.transaction_id, pCtx, !(--nMandatory+nOptional));
    str("nodes", pIe.nodes, pCtx, !(--nMandatory+nOptional));
    str("sequence_number", pIe.sequence_number, pCtx, !(--nMandatory+nOptional));
    pCtx = pCtx + "}";
    if (!pIsLast)
    {
        pCtx += ",";
    }
}

inline void encode_per(const list_all_request& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    encode_per(pIe.transaction_id, pCtx);
}

inline void decode_per(list_all_request& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    decode_per(pIe.transaction_id, pCtx);
}

inline void str(const char* pName, const list_all_request& pIe, std::string& pCtx, bool pIsLast)
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
    str("transaction_id", pIe.transaction_id, pCtx, !(--nMandatory+nOptional));
    pCtx = pCtx + "}";
    if (!pIsLast)
    {
        pCtx += ",";
    }
}

inline void encode_per(const list_all_response& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    encode_per(pIe.transaction_id, pCtx);
    encode_per(pIe.nodes, pCtx);
    encode_per(pIe.sequence_number, pCtx);
}

inline void decode_per(list_all_response& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    decode_per(pIe.transaction_id, pCtx);
    decode_per(pIe.nodes, pCtx);
    decode_per(pIe.sequence_number, pCtx);
}

inline void str(const char* pName, const list_all_response& pIe, std::string& pCtx, bool pIsLast)
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
    str("nodes", pIe.nodes, pCtx, !(--nMandatory+nOptional));
    str("sequence_number", pIe.sequence_number, pCtx, !(--nMandatory+nOptional));
    pCtx = pCtx + "}";
    if (!pIsLast)
    {
        pCtx += ",";
    }
}

inline void encode_per(const resolve_path_request& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    encode_per(pIe.transaction_id, pCtx);
    encode_per(pIe.origin, pCtx);
    encode_per(pIe.path, pCtx);
}

inline void decode_per(resolve_path_request& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    decode_per(pIe.transaction_id, pCtx);
    decode_per(pIe.origin, pCtx);
    decode_per(pIe.path, pCtx);
}

inline void str(const char* pName, const resolve_path_request& pIe, std::string& pCtx, bool pIsLast)
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
    str("origin", pIe.origin, pCtx, !(--nMandatory+nOptional));
    str("path", pIe.path, pCtx, !(--nMandatory+nOptional));
    pCtx = pCtx + "}";
    if (!pIsLast)
    {
        pCtx += ",";
    }
}

inline void encode_per(const resolve_path_response& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    uint8_t optionalmask[1] = {};
    if (pIe.node)
    {
        set_optional(optionalmask, 0);
    }
    encode_per(optionalmask, sizeof(optionalmask), pCtx);
    encode_per(pIe.transaction_id, pCtx);
    if (pIe.node)
    {
        encode_per(*pIe.node, pCtx);
    }
    encode_per(pIe.sequence_number, pCtx);
}

inline void decode_per(resolve_path_response& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    uint8_t optionalmask[1] = {};
    decode_per(optionalmask, sizeof(optionalmask), pCtx);
    decode_per(pIe.transaction_id, pCtx);
    if (check_optional(optionalmask, 0))
    {
        pIe.node = decltype(pIe.node)::value_type{};
        decode_per(*pIe.node, pCtx);
    }
    decode_per(pIe.sequence_number, pCtx);
}

inline void str(const char* pName, const resolve_path_response& pIe, std::string& pCtx, bool pIsLast)
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
    if (pIe.node) nOptional++;
    size_t nMandatory = 2;
    str("transaction_id", pIe.transaction_id, pCtx, !(--nMandatory+nOptional));
    if (pIe.node)
    {
        str("node", *pIe.node, pCtx, !(nMandatory+--nOptional));
    }
    str("sequence_number", pIe.sequence_number, pCtx, !(--nMandatory+nOptional));
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
    encode_per(pIe.id, pCtx);
    encode_per(pIe.name, pCtx);
}

inline void decode_per(attach& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    decode_per(pIe.transaction_id, pCtx);
    decode_per(pIe.parent, pCtx);
    decode_per(pIe.id, pCtx);
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
    str("id", pIe.id, pCtx, !(--nMandatory+nOptional));
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
    encode_per(pIe.id, pCtx);
}

inline void decode_per(detach& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    decode_per(pIe.transaction_id, pCtx);
    decode_per(pIe.id, pCtx);
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
    str("id", pIe.id, pCtx, !(--nMandatory+nOptional));
    pCtx = pCtx + "}";
    if (!pIsLast)
    {
        pCtx += ",";
    }
}

inline void encode_per(const node_subscribe& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    encode_per(pIe.id, pCtx);
}

inline void decode_per(node_subscribe& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    decode_per(pIe.id, pCtx);
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
    size_t nMandatory = 1;
    str("id", pIe.id, pCtx, !(--nMandatory+nOptional));
    pCtx = pCtx + "}";
    if (!pIsLast)
    {
        pCtx += ",";
    }
}

inline void encode_per(const node_unsubscribe& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    encode_per(pIe.id, pCtx);
}

inline void decode_per(node_unsubscribe& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    decode_per(pIe.id, pCtx);
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
    size_t nMandatory = 1;
    str("id", pIe.id, pCtx, !(--nMandatory+nOptional));
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
    encode_per(pIe.sequence_number, pCtx);
}

inline void decode_per(node_update_attach& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    decode_per(pIe.to_attach, pCtx);
    decode_per(pIe.sequence_number, pCtx);
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
    size_t nMandatory = 2;
    str("to_attach", pIe.to_attach, pCtx, !(--nMandatory+nOptional));
    str("sequence_number", pIe.sequence_number, pCtx, !(--nMandatory+nOptional));
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
    encode_per(pIe.sequence_number, pCtx);
}

inline void decode_per(node_update_detach& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    decode_per(pIe.to_detach, pCtx);
    decode_per(pIe.sequence_number, pCtx);
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
    size_t nMandatory = 2;
    str("to_detach", pIe.to_detach, pCtx, !(--nMandatory+nOptional));
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
}

inline void decode_per(protocol_value_server& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    using TypeIndex = uint8_t;
    TypeIndex type;
    decode_per(type, pCtx);
    if (0 == type)
    {
        pIe = subscribe();
        decode_per(std::get<0>(pIe), pCtx);
    }
    else if (1 == type)
    {
        pIe = unsubscribe();
        decode_per(std::get<1>(pIe), pCtx);
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
        std::string name = "subscribe";
        str(name.c_str(), std::get<0>(pIe), pCtx, true);
        pCtx += "}";
    }
    else if (1 == type)
    {
        if (pName)
            pCtx += std::string(pName) + ":{";
        else
            pCtx += "{";
        std::string name = "unsubscribe";
        str(name.c_str(), std::get<1>(pIe), pCtx, true);
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
}

inline void decode_per(protocol_value_client& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    using TypeIndex = uint8_t;
    TypeIndex type;
    decode_per(type, pCtx);
    if (0 == type)
    {
        pIe = update();
        decode_per(std::get<0>(pIe), pCtx);
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
        std::string name = "update";
        str(name.c_str(), std::get<0>(pIe), pCtx, true);
        pCtx += "}";
    }
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
    else if (5 == type)
    {
        encode_per(std::get<5>(pIe), pCtx);
    }
    else if (6 == type)
    {
        encode_per(std::get<6>(pIe), pCtx);
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
        pIe = list_all_request();
        decode_per(std::get<1>(pIe), pCtx);
    }
    else if (2 == type)
    {
        pIe = resolve_path_request();
        decode_per(std::get<2>(pIe), pCtx);
    }
    else if (3 == type)
    {
        pIe = attach();
        decode_per(std::get<3>(pIe), pCtx);
    }
    else if (4 == type)
    {
        pIe = detach();
        decode_per(std::get<4>(pIe), pCtx);
    }
    else if (5 == type)
    {
        pIe = node_subscribe();
        decode_per(std::get<5>(pIe), pCtx);
    }
    else if (6 == type)
    {
        pIe = node_unsubscribe();
        decode_per(std::get<6>(pIe), pCtx);
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
        std::string name = "list_all_request";
        str(name.c_str(), std::get<1>(pIe), pCtx, true);
        pCtx += "}";
    }
    else if (2 == type)
    {
        if (pName)
            pCtx += std::string(pName) + ":{";
        else
            pCtx += "{";
        std::string name = "resolve_path_request";
        str(name.c_str(), std::get<2>(pIe), pCtx, true);
        pCtx += "}";
    }
    else if (3 == type)
    {
        if (pName)
            pCtx += std::string(pName) + ":{";
        else
            pCtx += "{";
        std::string name = "attach";
        str(name.c_str(), std::get<3>(pIe), pCtx, true);
        pCtx += "}";
    }
    else if (4 == type)
    {
        if (pName)
            pCtx += std::string(pName) + ":{";
        else
            pCtx += "{";
        std::string name = "detach";
        str(name.c_str(), std::get<4>(pIe), pCtx, true);
        pCtx += "}";
    }
    else if (5 == type)
    {
        if (pName)
            pCtx += std::string(pName) + ":{";
        else
            pCtx += "{";
        std::string name = "node_subscribe";
        str(name.c_str(), std::get<5>(pIe), pCtx, true);
        pCtx += "}";
    }
    else if (6 == type)
    {
        if (pName)
            pCtx += std::string(pName) + ":{";
        else
            pCtx += "{";
        std::string name = "node_unsubscribe";
        str(name.c_str(), std::get<6>(pIe), pCtx, true);
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
    else if (4 == type)
    {
        encode_per(std::get<4>(pIe), pCtx);
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
        pIe = list_response();
        decode_per(std::get<0>(pIe), pCtx);
    }
    else if (1 == type)
    {
        pIe = list_all_response();
        decode_per(std::get<1>(pIe), pCtx);
    }
    else if (2 == type)
    {
        pIe = resolve_path_response();
        decode_per(std::get<2>(pIe), pCtx);
    }
    else if (3 == type)
    {
        pIe = node_update_attach();
        decode_per(std::get<3>(pIe), pCtx);
    }
    else if (4 == type)
    {
        pIe = node_update_detach();
        decode_per(std::get<4>(pIe), pCtx);
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
        std::string name = "list_response";
        str(name.c_str(), std::get<0>(pIe), pCtx, true);
        pCtx += "}";
    }
    else if (1 == type)
    {
        if (pName)
            pCtx += std::string(pName) + ":{";
        else
            pCtx += "{";
        std::string name = "list_all_response";
        str(name.c_str(), std::get<1>(pIe), pCtx, true);
        pCtx += "}";
    }
    else if (2 == type)
    {
        if (pName)
            pCtx += std::string(pName) + ":{";
        else
            pCtx += "{";
        std::string name = "resolve_path_response";
        str(name.c_str(), std::get<2>(pIe), pCtx, true);
        pCtx += "}";
    }
    else if (3 == type)
    {
        if (pName)
            pCtx += std::string(pName) + ":{";
        else
            pCtx += "{";
        std::string name = "node_update_attach";
        str(name.c_str(), std::get<3>(pIe), pCtx, true);
        pCtx += "}";
    }
    else if (4 == type)
    {
        if (pName)
            pCtx += std::string(pName) + ":{";
        else
            pCtx += "{";
        std::string name = "node_update_detach";
        str(name.c_str(), std::get<4>(pIe), pCtx, true);
        pCtx += "}";
    }
    if (!pIsLast)
    {
        pCtx += ",";
    }
}

} // namespace cum
#endif //__CUM_MSG_HPP__
