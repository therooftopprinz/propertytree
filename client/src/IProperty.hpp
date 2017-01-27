#ifndef CLIENT_IPROPERTY_HPP_
#define CLIENT_IPROPERTY_HPP_

#include <string>
#include <interface/protocol.hpp>

namespace ptree
{
namespace client
{

struct IProperty
{
public:
    IProperty(protocol::Uuid uuid, std::string path, protocol::PropertyType type, bool owned):
        uuid(uuid), path(path), type(type), owned(owned)
    {
    }

    virtual protocol::Uuid getUuid()
    {
        return uuid;
    }

    virtual const std::string& getPath()
    {
        return path;
    }

    virtual protocol::PropertyType getType()
    {
        return type;
    }

    virtual bool isOwned()
    {
        return owned;
    }
private:
    protocol::Uuid uuid;
    std::string path;
    protocol::PropertyType type;
    bool owned;
};

}
}
#endif  // CLIENT_IPROPERTY_HPP_
