
#ifndef INTERFACE_PROTOOCOLX_HPP_
#define INTERFACE_PROTOOCOLX_HPP_

#include <cstdint>
#include <memory>
#include "MessageEssential.hpp"

namespace ptree
{

namespace protocol
{

using protocol::Uuid;
using protocol::PropertyType;
using protocol::MessageType;
using protocol::MessageHeader;

//////////////////

typedef std::shared_ptr<MessageHeader> MessageHeaderPtr;

} // namespace protocol
} // namespace ptree


// Notes:
// CreateResponse::response size

#endif  // INTERFACE_PROTOOCOL_HPP_