#ifndef __PROPERTYTREE_PROTOCOL_HPP__
#define __PROPERTYTREE_PROTOCOL_HPP__

#include <cstdint>
#include <limits>

namespace propertytree
{

constexpr uint64_t TRANSACTION_INDICATION = std::numeric_limits<uint64_t>::max();

enum status_e
{
    E_STATUS_OK,
    E_STATUS_DATA_TOO_BIG
};

enum message_type_e
{
    E_MSGT_ACKNOWLEDGE,
    E_MSGT_SET,
    E_MSGT_GET,
    E_MSGT_VALUE,
    E_MSGT_SUBSCRIBE,
    E_MSGT_UNSUBSCRIBE
};

constexpr uint8_t HEADER_FLAG_EXTENSION     = (1 << 7);
constexpr uint8_t HEADER_FLAG_TRANSACTIONAL = (1 << 0);

struct header_s
{
    uint8_t  type;
    uint8_t  flags;
    union
    {
        uint16_t transaction;
        uint16_t spare3;
    };
    uint32_t spare2;
};

// MSG ACKNOWLEDGE
struct acknowledge_s
{
    uint64_t status;
};

// MSG STREAM
// MSG SET
// MSG GET
// MSG SUBSCRIBE
// MSG UNSUBSCRIBE
struct key_s
{
    uint16_t key;
};

// MSG VALUE
struct key_sn_s
{
    uint16_t key;
    uint16_t sn;
};

template <typename T, typename U>
inline T* next_struct(U* s)
{
    return (T*)((std::byte*)(s)+sizeof(U));
};

template <typename T, typename U>
inline const T* next_struct(const U* s)
{
    return (T*)((std::byte*)(s)+sizeof(U));
};

} // namespace propertytree

#endif // __PROPERTYTREE_PROTOCOL_HPP__
