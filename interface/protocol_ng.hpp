// Type:  ('buf', {'type': 'u8'})
// Type:  ('buf', {'dynamic_array': '256'})
// Type:  ('u64_list', {'type': 'u64'})
// Type:  ('u64_list', {'dynamic_array': '256'})
// Type:  ('string_list', {'type': 'string'})
// Type:  ('string_list', {'dynamic_array': '256'})
// Enumeration:  ('status_code', ('OK', None))
// Enumeration:  ('status_code', ('NOT_FOUND', None))
// Enumeration:  ('status_code', ('PARENT_NOT_FOUND', None))
// Enumeration:  ('status_code', ('NAME_EXISTS', None))
// Sequence:  value ('u64', 'id')
// Sequence:  value ('buf', 'data')
// Type:  ('value_list', {'type': 'value'})
// Type:  ('value_list', {'dynamic_array': '256'})
// Sequence:  allocate_request ('u16', 'trId')
// Sequence:  allocate_response ('u16', 'trId')
// Sequence:  allocate_response ('u64', 'id')
// Type:  ('set_value', {'type': 'value'})
// Sequence:  get_value_request ('u16', 'trId')
// Sequence:  get_value_request ('u64', 'id')
// Sequence:  get_value_response ('u16', 'trId')
// Sequence:  get_value_response ('value', 'data')
// Sequence:  get_value_response ('u64', 'sn')
// Sequence:  subscribe ('u64', 'id')
// Sequence:  subscribe ('u64', 'interval')
// Sequence:  unsubscribe ('u64', 'id')
// Sequence:  update ('value_list', 'values')
// Sequence:  update ('u64', 'sn')
// Sequence:  node ('u64', 'id')
// Sequence:  node ('u64', 'parent')
// Sequence:  node ('string', 'name')
// Type:  ('node_list', {'type': 'node'})
// Type:  ('node_list', {'dynamic_array': '4294967296'})
// Type:  ('node_optional', {'type': 'node'})
// Sequence:  list_request ('u16', 'trId')
// Sequence:  list_request ('u64', 'id')
// Sequence:  list_response ('u16', 'trId')
// Sequence:  list_response ('node_list', 'nodes')
// Sequence:  list_response ('u64', 'sn')
// Sequence:  resolve_path_request ('u16', 'trId')
// Sequence:  resolve_path_request ('u64', 'origin')
// Sequence:  resolve_path_request ('string_list', 'path')
// Sequence:  resolve_path_response ('u16', 'trId')
// Sequence:  resolve_path_response ('node_optional', 'node')
// Sequence:  resolve_path_response ('u64', 'sn')
// Sequence:  aquire_node_request ('u16', 'trId')
// Sequence:  aquire_node_request ('u64', 'id')
// Sequence:  aquire_node_response ('u16', 'trId')
// Sequence:  aquire_node_response ('node_optional', 'node')
// Sequence:  aquire_node_response ('u64', 'sn')
// Sequence:  release_node_request ('u16', 'trId')
// Sequence:  release_node_request ('u64', 'id')
// Sequence:  release_node_response ('u16', 'trId')
// Sequence:  release_node_response ('status_code', 'status')
// Sequence:  node_action_add ('node', 'to_add')
// Sequence:  node_action_delete ('u64', 'to_delete')
// Sequence:  node_action_update ('node', 'to_update')
// Choice:  ('node_action', 'node_action_add')
// Choice:  ('node_action', 'node_action_delete')
// Choice:  ('node_action', 'node_action_update')
// Sequence:  node_update ('node_action', 'update')
// Sequence:  node_update ('u64', 'sn')
// Sequence:  attach_request ('u16', 'trId')
// Sequence:  attach_request ('u64', 'parent')
// Sequence:  attach_request ('u64', 'id')
// Sequence:  attach_request ('string', 'name')
// Sequence:  attach_response ('u16', 'trId')
// Sequence:  attach_response ('status_code', 'status')
// Choice:  ('protocol_value', 'allocate_request')
// Choice:  ('protocol_value', 'allocate_response')
// Choice:  ('protocol_value', 'set_value')
// Choice:  ('protocol_value', 'get_value_request')
// Choice:  ('protocol_value', 'get_value_response')
// Choice:  ('protocol_value', 'subscribe')
// Choice:  ('protocol_value', 'unsubscribe')
// Choice:  ('protocol_value', 'update')
// Choice:  ('protocol_node', 'list_request')
// Choice:  ('protocol_node', 'list_response')
// Choice:  ('protocol_node', 'resolve_path_request')
// Choice:  ('protocol_node', 'resolve_path_response')
// Choice:  ('protocol_node', 'aquire_node_request')
// Choice:  ('protocol_node', 'aquire_node_response')
// Choice:  ('protocol_node', 'release_node_request')
// Choice:  ('protocol_node', 'release_node_response')
// Choice:  ('protocol_node', 'node_update')
// Choice:  ('protocol_node', 'attach_request')
// Choice:  ('protocol_node', 'attach_response')
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
enum status_code
{
    OK,
    NOT_FOUND,
    PARENT_NOT_FOUND,
    NAME_EXISTS
};

struct value
{
    u64 id;
    buf data;
};

using value_list = cum::vector<value, 256>;
struct allocate_request
{
    u16 trId;
};

struct allocate_response
{
    u16 trId;
    u64 id;
};

using set_value = value;
struct get_value_request
{
    u16 trId;
    u64 id;
};

struct get_value_response
{
    u16 trId;
    value data;
    u64 sn;
};

struct subscribe
{
    u64 id;
    u64 interval;
};

struct unsubscribe
{
    u64 id;
};

struct update
{
    value_list values;
    u64 sn;
};

struct node
{
    u64 id;
    u64 parent;
    string name;
};

using node_list = cum::vector<node, 4294967296>;
using node_optional = node;
struct list_request
{
    u16 trId;
    u64 id;
};

struct list_response
{
    u16 trId;
    node_list nodes;
    u64 sn;
};

struct resolve_path_request
{
    u16 trId;
    u64 origin;
    string_list path;
};

struct resolve_path_response
{
    u16 trId;
    node_optional node;
    u64 sn;
};

struct aquire_node_request
{
    u16 trId;
    u64 id;
};

struct aquire_node_response
{
    u16 trId;
    node_optional node;
    u64 sn;
};

struct release_node_request
{
    u16 trId;
    u64 id;
};

struct release_node_response
{
    u16 trId;
    status_code status;
};

struct node_action_add
{
    node to_add;
};

struct node_action_delete
{
    u64 to_delete;
};

struct node_action_update
{
    node to_update;
};

using node_action = std::variant<node_action_add,node_action_delete,node_action_update>;
struct node_update
{
    node_action update;
    u64 sn;
};

struct attach_request
{
    u16 trId;
    u64 parent;
    u64 id;
    string name;
};

struct attach_response
{
    u16 trId;
    status_code status;
};

using protocol_value = std::variant<allocate_request,allocate_response,set_value,get_value_request,get_value_response,subscribe,unsubscribe,update>;
using protocol_node = std::variant<list_request,list_response,resolve_path_request,resolve_path_response,aquire_node_request,aquire_node_response,release_node_request,release_node_response,node_update,attach_request,attach_response>;
/***********************************************
/
/            Codec Definitions
/
************************************************/

inline void str(const char* pName, const status_code& pIe, std::string& pCtx, bool pIsLast)
{
    using namespace cum;
    if (pName)
    {
        pCtx = pCtx + "\"" + pName + "\":";
    }
    if (status_code::OK == pIe) pCtx += "\"OK\"";
    if (status_code::NOT_FOUND == pIe) pCtx += "\"NOT_FOUND\"";
    if (status_code::PARENT_NOT_FOUND == pIe) pCtx += "\"PARENT_NOT_FOUND\"";
    if (status_code::NAME_EXISTS == pIe) pCtx += "\"NAME_EXISTS\"";
    pCtx = pCtx + "}";
    if (!pIsLast)
    {
        pCtx += ",";
    }
}

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

inline void encode_per(const allocate_request& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    encode_per(pIe.trId, pCtx);
}

inline void decode_per(allocate_request& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    decode_per(pIe.trId, pCtx);
}

inline void str(const char* pName, const allocate_request& pIe, std::string& pCtx, bool pIsLast)
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
    str("trId", pIe.trId, pCtx, !(--nMandatory+nOptional));
    pCtx = pCtx + "}";
    if (!pIsLast)
    {
        pCtx += ",";
    }
}

inline void encode_per(const allocate_response& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    encode_per(pIe.trId, pCtx);
    encode_per(pIe.id, pCtx);
}

inline void decode_per(allocate_response& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    decode_per(pIe.trId, pCtx);
    decode_per(pIe.id, pCtx);
}

inline void str(const char* pName, const allocate_response& pIe, std::string& pCtx, bool pIsLast)
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
    str("trId", pIe.trId, pCtx, !(--nMandatory+nOptional));
    str("id", pIe.id, pCtx, !(--nMandatory+nOptional));
    pCtx = pCtx + "}";
    if (!pIsLast)
    {
        pCtx += ",";
    }
}

inline void encode_per(const get_value_request& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    encode_per(pIe.trId, pCtx);
    encode_per(pIe.id, pCtx);
}

inline void decode_per(get_value_request& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    decode_per(pIe.trId, pCtx);
    decode_per(pIe.id, pCtx);
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
    str("trId", pIe.trId, pCtx, !(--nMandatory+nOptional));
    str("id", pIe.id, pCtx, !(--nMandatory+nOptional));
    pCtx = pCtx + "}";
    if (!pIsLast)
    {
        pCtx += ",";
    }
}

inline void encode_per(const get_value_response& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    encode_per(pIe.trId, pCtx);
    encode_per(pIe.data, pCtx);
    encode_per(pIe.sn, pCtx);
}

inline void decode_per(get_value_response& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    decode_per(pIe.trId, pCtx);
    decode_per(pIe.data, pCtx);
    decode_per(pIe.sn, pCtx);
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
    str("trId", pIe.trId, pCtx, !(--nMandatory+nOptional));
    str("data", pIe.data, pCtx, !(--nMandatory+nOptional));
    str("sn", pIe.sn, pCtx, !(--nMandatory+nOptional));
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
    encode_per(pIe.interval, pCtx);
}

inline void decode_per(subscribe& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    decode_per(pIe.id, pCtx);
    decode_per(pIe.interval, pCtx);
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
    str("id", pIe.id, pCtx, !(--nMandatory+nOptional));
    str("interval", pIe.interval, pCtx, !(--nMandatory+nOptional));
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
    encode_per(pIe.values, pCtx);
    encode_per(pIe.sn, pCtx);
}

inline void decode_per(update& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    decode_per(pIe.values, pCtx);
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
    str("values", pIe.values, pCtx, !(--nMandatory+nOptional));
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
    encode_per(pIe.trId, pCtx);
    encode_per(pIe.id, pCtx);
}

inline void decode_per(list_request& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    decode_per(pIe.trId, pCtx);
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
    str("trId", pIe.trId, pCtx, !(--nMandatory+nOptional));
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
    encode_per(pIe.trId, pCtx);
    encode_per(pIe.nodes, pCtx);
    encode_per(pIe.sn, pCtx);
}

inline void decode_per(list_response& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    decode_per(pIe.trId, pCtx);
    decode_per(pIe.nodes, pCtx);
    decode_per(pIe.sn, pCtx);
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
    str("trId", pIe.trId, pCtx, !(--nMandatory+nOptional));
    str("nodes", pIe.nodes, pCtx, !(--nMandatory+nOptional));
    str("sn", pIe.sn, pCtx, !(--nMandatory+nOptional));
    pCtx = pCtx + "}";
    if (!pIsLast)
    {
        pCtx += ",";
    }
}

inline void encode_per(const resolve_path_request& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    encode_per(pIe.trId, pCtx);
    encode_per(pIe.origin, pCtx);
    encode_per(pIe.path, pCtx);
}

inline void decode_per(resolve_path_request& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    decode_per(pIe.trId, pCtx);
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
    str("trId", pIe.trId, pCtx, !(--nMandatory+nOptional));
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
    encode_per(pIe.trId, pCtx);
    encode_per(pIe.node, pCtx);
    encode_per(pIe.sn, pCtx);
}

inline void decode_per(resolve_path_response& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    decode_per(pIe.trId, pCtx);
    decode_per(pIe.node, pCtx);
    decode_per(pIe.sn, pCtx);
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
    size_t nMandatory = 3;
    str("trId", pIe.trId, pCtx, !(--nMandatory+nOptional));
    str("node", pIe.node, pCtx, !(--nMandatory+nOptional));
    str("sn", pIe.sn, pCtx, !(--nMandatory+nOptional));
    pCtx = pCtx + "}";
    if (!pIsLast)
    {
        pCtx += ",";
    }
}

inline void encode_per(const aquire_node_request& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    encode_per(pIe.trId, pCtx);
    encode_per(pIe.id, pCtx);
}

inline void decode_per(aquire_node_request& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    decode_per(pIe.trId, pCtx);
    decode_per(pIe.id, pCtx);
}

inline void str(const char* pName, const aquire_node_request& pIe, std::string& pCtx, bool pIsLast)
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
    str("trId", pIe.trId, pCtx, !(--nMandatory+nOptional));
    str("id", pIe.id, pCtx, !(--nMandatory+nOptional));
    pCtx = pCtx + "}";
    if (!pIsLast)
    {
        pCtx += ",";
    }
}

inline void encode_per(const aquire_node_response& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    encode_per(pIe.trId, pCtx);
    encode_per(pIe.node, pCtx);
    encode_per(pIe.sn, pCtx);
}

inline void decode_per(aquire_node_response& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    decode_per(pIe.trId, pCtx);
    decode_per(pIe.node, pCtx);
    decode_per(pIe.sn, pCtx);
}

inline void str(const char* pName, const aquire_node_response& pIe, std::string& pCtx, bool pIsLast)
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
    str("trId", pIe.trId, pCtx, !(--nMandatory+nOptional));
    str("node", pIe.node, pCtx, !(--nMandatory+nOptional));
    str("sn", pIe.sn, pCtx, !(--nMandatory+nOptional));
    pCtx = pCtx + "}";
    if (!pIsLast)
    {
        pCtx += ",";
    }
}

inline void encode_per(const release_node_request& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    encode_per(pIe.trId, pCtx);
    encode_per(pIe.id, pCtx);
}

inline void decode_per(release_node_request& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    decode_per(pIe.trId, pCtx);
    decode_per(pIe.id, pCtx);
}

inline void str(const char* pName, const release_node_request& pIe, std::string& pCtx, bool pIsLast)
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
    str("trId", pIe.trId, pCtx, !(--nMandatory+nOptional));
    str("id", pIe.id, pCtx, !(--nMandatory+nOptional));
    pCtx = pCtx + "}";
    if (!pIsLast)
    {
        pCtx += ",";
    }
}

inline void encode_per(const release_node_response& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    encode_per(pIe.trId, pCtx);
    encode_per(pIe.status, pCtx);
}

inline void decode_per(release_node_response& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    decode_per(pIe.trId, pCtx);
    decode_per(pIe.status, pCtx);
}

inline void str(const char* pName, const release_node_response& pIe, std::string& pCtx, bool pIsLast)
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
    str("trId", pIe.trId, pCtx, !(--nMandatory+nOptional));
    str("status", pIe.status, pCtx, !(--nMandatory+nOptional));
    pCtx = pCtx + "}";
    if (!pIsLast)
    {
        pCtx += ",";
    }
}

inline void encode_per(const node_action_add& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    encode_per(pIe.to_add, pCtx);
}

inline void decode_per(node_action_add& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    decode_per(pIe.to_add, pCtx);
}

inline void str(const char* pName, const node_action_add& pIe, std::string& pCtx, bool pIsLast)
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
    str("to_add", pIe.to_add, pCtx, !(--nMandatory+nOptional));
    pCtx = pCtx + "}";
    if (!pIsLast)
    {
        pCtx += ",";
    }
}

inline void encode_per(const node_action_delete& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    encode_per(pIe.to_delete, pCtx);
}

inline void decode_per(node_action_delete& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    decode_per(pIe.to_delete, pCtx);
}

inline void str(const char* pName, const node_action_delete& pIe, std::string& pCtx, bool pIsLast)
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
    str("to_delete", pIe.to_delete, pCtx, !(--nMandatory+nOptional));
    pCtx = pCtx + "}";
    if (!pIsLast)
    {
        pCtx += ",";
    }
}

inline void encode_per(const node_action_update& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    encode_per(pIe.to_update, pCtx);
}

inline void decode_per(node_action_update& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    decode_per(pIe.to_update, pCtx);
}

inline void str(const char* pName, const node_action_update& pIe, std::string& pCtx, bool pIsLast)
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
    str("to_update", pIe.to_update, pCtx, !(--nMandatory+nOptional));
    pCtx = pCtx + "}";
    if (!pIsLast)
    {
        pCtx += ",";
    }
}

inline void encode_per(const node_action& pIe, cum::per_codec_ctx& pCtx)
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

inline void decode_per(node_action& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    using TypeIndex = uint8_t;
    TypeIndex type;
    decode_per(type, pCtx);
    if (0 == type)
    {
        pIe = node_action_add();
        decode_per(std::get<0>(pIe), pCtx);
    }
    else if (1 == type)
    {
        pIe = node_action_delete();
        decode_per(std::get<1>(pIe), pCtx);
    }
    else if (2 == type)
    {
        pIe = node_action_update();
        decode_per(std::get<2>(pIe), pCtx);
    }
}

inline void str(const char* pName, const node_action& pIe, std::string& pCtx, bool pIsLast)
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
        std::string name = "node_action_add";
        str(name.c_str(), std::get<0>(pIe), pCtx, true);
        pCtx += "}";
    }
    else if (1 == type)
    {
        if (pName)
            pCtx += std::string(pName) + ":{";
        else
            pCtx += "{";
        std::string name = "node_action_delete";
        str(name.c_str(), std::get<1>(pIe), pCtx, true);
        pCtx += "}";
    }
    else if (2 == type)
    {
        if (pName)
            pCtx += std::string(pName) + ":{";
        else
            pCtx += "{";
        std::string name = "node_action_update";
        str(name.c_str(), std::get<2>(pIe), pCtx, true);
        pCtx += "}";
    }
    if (!pIsLast)
    {
        pCtx += ",";
    }
}

inline void encode_per(const node_update& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    encode_per(pIe.update, pCtx);
    encode_per(pIe.sn, pCtx);
}

inline void decode_per(node_update& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    decode_per(pIe.update, pCtx);
    decode_per(pIe.sn, pCtx);
}

inline void str(const char* pName, const node_update& pIe, std::string& pCtx, bool pIsLast)
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
    str("update", pIe.update, pCtx, !(--nMandatory+nOptional));
    str("sn", pIe.sn, pCtx, !(--nMandatory+nOptional));
    pCtx = pCtx + "}";
    if (!pIsLast)
    {
        pCtx += ",";
    }
}

inline void encode_per(const attach_request& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    encode_per(pIe.trId, pCtx);
    encode_per(pIe.parent, pCtx);
    encode_per(pIe.id, pCtx);
    encode_per(pIe.name, pCtx);
}

inline void decode_per(attach_request& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    decode_per(pIe.trId, pCtx);
    decode_per(pIe.parent, pCtx);
    decode_per(pIe.id, pCtx);
    decode_per(pIe.name, pCtx);
}

inline void str(const char* pName, const attach_request& pIe, std::string& pCtx, bool pIsLast)
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
    str("trId", pIe.trId, pCtx, !(--nMandatory+nOptional));
    str("parent", pIe.parent, pCtx, !(--nMandatory+nOptional));
    str("id", pIe.id, pCtx, !(--nMandatory+nOptional));
    str("name", pIe.name, pCtx, !(--nMandatory+nOptional));
    pCtx = pCtx + "}";
    if (!pIsLast)
    {
        pCtx += ",";
    }
}

inline void encode_per(const attach_response& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    encode_per(pIe.trId, pCtx);
    encode_per(pIe.status, pCtx);
}

inline void decode_per(attach_response& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    decode_per(pIe.trId, pCtx);
    decode_per(pIe.status, pCtx);
}

inline void str(const char* pName, const attach_response& pIe, std::string& pCtx, bool pIsLast)
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
    str("trId", pIe.trId, pCtx, !(--nMandatory+nOptional));
    str("status", pIe.status, pCtx, !(--nMandatory+nOptional));
    pCtx = pCtx + "}";
    if (!pIsLast)
    {
        pCtx += ",";
    }
}

inline void encode_per(const protocol_value& pIe, cum::per_codec_ctx& pCtx)
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
}

inline void decode_per(protocol_value& pIe, cum::per_codec_ctx& pCtx)
{
    using namespace cum;
    using TypeIndex = uint8_t;
    TypeIndex type;
    decode_per(type, pCtx);
    if (0 == type)
    {
        pIe = allocate_request();
        decode_per(std::get<0>(pIe), pCtx);
    }
    else if (1 == type)
    {
        pIe = allocate_response();
        decode_per(std::get<1>(pIe), pCtx);
    }
    else if (2 == type)
    {
        pIe = set_value();
        decode_per(std::get<2>(pIe), pCtx);
    }
    else if (3 == type)
    {
        pIe = get_value_request();
        decode_per(std::get<3>(pIe), pCtx);
    }
    else if (4 == type)
    {
        pIe = get_value_response();
        decode_per(std::get<4>(pIe), pCtx);
    }
    else if (5 == type)
    {
        pIe = subscribe();
        decode_per(std::get<5>(pIe), pCtx);
    }
    else if (6 == type)
    {
        pIe = unsubscribe();
        decode_per(std::get<6>(pIe), pCtx);
    }
    else if (7 == type)
    {
        pIe = update();
        decode_per(std::get<7>(pIe), pCtx);
    }
}

inline void str(const char* pName, const protocol_value& pIe, std::string& pCtx, bool pIsLast)
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
        std::string name = "allocate_request";
        str(name.c_str(), std::get<0>(pIe), pCtx, true);
        pCtx += "}";
    }
    else if (1 == type)
    {
        if (pName)
            pCtx += std::string(pName) + ":{";
        else
            pCtx += "{";
        std::string name = "allocate_response";
        str(name.c_str(), std::get<1>(pIe), pCtx, true);
        pCtx += "}";
    }
    else if (2 == type)
    {
        if (pName)
            pCtx += std::string(pName) + ":{";
        else
            pCtx += "{";
        std::string name = "set_value";
        str(name.c_str(), std::get<2>(pIe), pCtx, true);
        pCtx += "}";
    }
    else if (3 == type)
    {
        if (pName)
            pCtx += std::string(pName) + ":{";
        else
            pCtx += "{";
        std::string name = "get_value_request";
        str(name.c_str(), std::get<3>(pIe), pCtx, true);
        pCtx += "}";
    }
    else if (4 == type)
    {
        if (pName)
            pCtx += std::string(pName) + ":{";
        else
            pCtx += "{";
        std::string name = "get_value_response";
        str(name.c_str(), std::get<4>(pIe), pCtx, true);
        pCtx += "}";
    }
    else if (5 == type)
    {
        if (pName)
            pCtx += std::string(pName) + ":{";
        else
            pCtx += "{";
        std::string name = "subscribe";
        str(name.c_str(), std::get<5>(pIe), pCtx, true);
        pCtx += "}";
    }
    else if (6 == type)
    {
        if (pName)
            pCtx += std::string(pName) + ":{";
        else
            pCtx += "{";
        std::string name = "unsubscribe";
        str(name.c_str(), std::get<6>(pIe), pCtx, true);
        pCtx += "}";
    }
    else if (7 == type)
    {
        if (pName)
            pCtx += std::string(pName) + ":{";
        else
            pCtx += "{";
        std::string name = "update";
        str(name.c_str(), std::get<7>(pIe), pCtx, true);
        pCtx += "}";
    }
    if (!pIsLast)
    {
        pCtx += ",";
    }
}

inline void encode_per(const protocol_node& pIe, cum::per_codec_ctx& pCtx)
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
}

inline void decode_per(protocol_node& pIe, cum::per_codec_ctx& pCtx)
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
        pIe = list_response();
        decode_per(std::get<1>(pIe), pCtx);
    }
    else if (2 == type)
    {
        pIe = resolve_path_request();
        decode_per(std::get<2>(pIe), pCtx);
    }
    else if (3 == type)
    {
        pIe = resolve_path_response();
        decode_per(std::get<3>(pIe), pCtx);
    }
    else if (4 == type)
    {
        pIe = aquire_node_request();
        decode_per(std::get<4>(pIe), pCtx);
    }
    else if (5 == type)
    {
        pIe = aquire_node_response();
        decode_per(std::get<5>(pIe), pCtx);
    }
    else if (6 == type)
    {
        pIe = release_node_request();
        decode_per(std::get<6>(pIe), pCtx);
    }
    else if (7 == type)
    {
        pIe = release_node_response();
        decode_per(std::get<7>(pIe), pCtx);
    }
    else if (8 == type)
    {
        pIe = node_update();
        decode_per(std::get<8>(pIe), pCtx);
    }
    else if (9 == type)
    {
        pIe = attach_request();
        decode_per(std::get<9>(pIe), pCtx);
    }
    else if (10 == type)
    {
        pIe = attach_response();
        decode_per(std::get<10>(pIe), pCtx);
    }
}

inline void str(const char* pName, const protocol_node& pIe, std::string& pCtx, bool pIsLast)
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
        std::string name = "resolve_path_response";
        str(name.c_str(), std::get<3>(pIe), pCtx, true);
        pCtx += "}";
    }
    else if (4 == type)
    {
        if (pName)
            pCtx += std::string(pName) + ":{";
        else
            pCtx += "{";
        std::string name = "aquire_node_request";
        str(name.c_str(), std::get<4>(pIe), pCtx, true);
        pCtx += "}";
    }
    else if (5 == type)
    {
        if (pName)
            pCtx += std::string(pName) + ":{";
        else
            pCtx += "{";
        std::string name = "aquire_node_response";
        str(name.c_str(), std::get<5>(pIe), pCtx, true);
        pCtx += "}";
    }
    else if (6 == type)
    {
        if (pName)
            pCtx += std::string(pName) + ":{";
        else
            pCtx += "{";
        std::string name = "release_node_request";
        str(name.c_str(), std::get<6>(pIe), pCtx, true);
        pCtx += "}";
    }
    else if (7 == type)
    {
        if (pName)
            pCtx += std::string(pName) + ":{";
        else
            pCtx += "{";
        std::string name = "release_node_response";
        str(name.c_str(), std::get<7>(pIe), pCtx, true);
        pCtx += "}";
    }
    else if (8 == type)
    {
        if (pName)
            pCtx += std::string(pName) + ":{";
        else
            pCtx += "{";
        std::string name = "node_update";
        str(name.c_str(), std::get<8>(pIe), pCtx, true);
        pCtx += "}";
    }
    else if (9 == type)
    {
        if (pName)
            pCtx += std::string(pName) + ":{";
        else
            pCtx += "{";
        std::string name = "attach_request";
        str(name.c_str(), std::get<9>(pIe), pCtx, true);
        pCtx += "}";
    }
    else if (10 == type)
    {
        if (pName)
            pCtx += std::string(pName) + ":{";
        else
            pCtx += "{";
        std::string name = "attach_response";
        str(name.c_str(), std::get<10>(pIe), pCtx, true);
        pCtx += "}";
    }
    if (!pIsLast)
    {
        pCtx += ",";
    }
}

} // namespace cum
#endif //__CUM_MSG_HPP__
