
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

//////////////////

typedef std::shared_ptr<MessageHeader> MessageHeaderPtr;

} // namespace protocol
} // namespace ptree


// Notes:
// CreateResponse::response size

#endif  // INTERFACE_PROTOOCOL_HPP_