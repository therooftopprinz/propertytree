#ifndef CLIENT_NODECONTAINER_HPP_
#define CLIENT_NODECONTAINER_HPP_

#include <cassert>
#include <map>
#include <vector>
#include <set>
#include <string>
#include <cstring>
#include <mutex>
#include <thread>
#include <memory>

#include <interface/protocol.hpp>
#include <common/src/Logger.hpp>
#include <client/src/Types.hpp>
#include <client/src/IProperty.hpp>

namespace ptree
{
namespace client
{

class NodeContainer : public IProperty, public std::enable_shared_from_this<NodeContainer>
{
public:

    NodeContainer(protocol::Uuid uuid, std::string path, bool owned):
        IProperty(uuid, path, protocol::PropertyType::Node, owned)
    {
    }
};

}
}

#endif // CLIENT_NODECONTAINER_HPP_
