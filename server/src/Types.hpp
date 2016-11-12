#include <memory>
#include <vector>
#include <map>

#ifndef SERVER_TYPES_HPP_
#define SERVER_TYPES_HPP_

namespace ptree
{
namespace server
{
    class IClientServerMonitor;
    class IClientServer;
    class ClientServer;


    typedef std::shared_ptr<ClientServer> ClientServerPtr;
    typedef std::weak_ptr<ClientServer> ClientServerWkPtr;
    typedef std::shared_ptr<IClientServerMonitor> IClientServerMonitorPtr;
    
    typedef std::vector<uint8_t> Buffer;
    typedef std::shared_ptr<Buffer> BufferPtr;

    class IEndPoint;
    class EndPoint;

    typedef std::shared_ptr<IEndPoint> IEndPointPtr;
    typedef std::shared_ptr<EndPoint> EndPointPtr;

} // namespace server

namespace core
{
    class Node;
    class Value;
    class Rpc;

    typedef std::shared_ptr<Node> NodePtr;
    typedef std::weak_ptr<Node> NodeWkPtr;
    typedef std::shared_ptr<Value> ValuePtr;
    typedef std::weak_ptr<Value> ValueWkPtr;

    class IProperty;

    typedef std::shared_ptr<IProperty> IPropertyPtr;
    typedef std::vector<uint8_t> ValueContainer;
    typedef std::shared_ptr<ValueContainer> ValueContainerPtr;
    typedef std::function<bool(ValuePtr)> ValueWatcher;
    
    class PTree;
    class IdGenerator;
    typedef std::pair<void*, ValueWatcher> IdWatcherPair;
    typedef std::shared_ptr<IdGenerator> IIdGeneratorPtr;
    typedef std::map<std::string, IPropertyPtr> PropertyMap;
    typedef std::shared_ptr<PropertyMap> PropertyMapPtr;
    typedef std::shared_ptr<PTree> PTreePtr;

} // namespace core
} // namespace ptree

#endif // SERVER_TYPES_HPP_