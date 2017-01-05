#ifndef INTERFACE_PROTOOCOL_HPP_
#define INTERFACE_PROTOOCOL_HPP_

#include "MessageCoDec.hpp"
#include "MessageCoDecHelpers.hpp"

namespace ptree
{

namespace protocol_x
{

struct SigninRequest
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

    MESSAGE_FIELDS_PROTOX(version, refreshRate, featureFlag);
};

struct SigninResponse
{
    uint32_t version;
    MESSAGE_FIELDS_PROTOX(version);
};


} // namespace protocol
} // namespace ptree

#include "protocol_x.hpp"

#endif  // INTERFACE_PROTOOCOL_HPP_
