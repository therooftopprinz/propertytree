#ifndef __PROPERTY_HPP__
#define __PROPERTY_HPP__

#include <utility>

#include <bfc/EpollReactor.hpp>

#include <propertytree/Node.hpp>
#include <propertytree/Client.hpp>

namespace propertytree
{

class Property
{
public:
    Property() = delete;

    template <typename T>
    Property& operator=(const T& pOther)
    {
        std::unique_lock<std::mutex> lg(mNode->dataMutex);
        auto& node = *mNode;
        if (sizeof(pOther) == node.data.size())
        {
            new (node.data.data()) T(pOther);
        }
        else
        {
            auto raw = new std::byte[sizeof(T)];
            new (raw) T(pOther);
            node.data = bfc::Buffer((std::byte*)raw, sizeof(T)); 
        }

        mClient.commit(*this);
        return *this;
    }

    template <typename T>
    std::optional<T> value()
    {
        std::unique_lock<std::mutex> lg(mNode->dataMutex);
        if (sizeof(T) == mNode->data.size())
        {
            return *((T*)mNode->data.data());
        }
        return {};
    }

    void fetch()
    {
        mClient.fetch(*this);
    }

    Property create(const std::string& pName)
    {
        return mClient.create(*this, pName);
    }

    Property get(const std::string& pName)
    {
        return mClient.get(*this, pName, false);
    }

    operator bool() const
    {
        return mNode ? true : false;
    }

    void subscribe()
    {
        mClient.subscribe(*this);
    }

    void unsubscribe()
    {
        mClient.unsubscribe(*this);
    }

    void destroy()
    {
        mClient.destroy(*this);
    }

    void loadChildren(bool pRecursive = false)
    {
        mClient.get(*this, ".", true);
    }

    std::vector<std::pair<std::string, Property>> children()
    {
        std::vector<std::pair<std::string, Property>> rv;
        std::unique_lock<std::mutex> lg(mNode->childrenMutex);
        for (auto& i : mNode->children)
        {
            rv.emplace_back(i.first, Property(mClient, i.second));
        }
        return rv;
    }

    size_t childrenSize()
    {
        std::unique_lock<std::mutex> lg(mNode->childrenMutex);
        return mNode->children.size();
    }

    bfc::Buffer call(const bfc::BufferView& pValue)
    {
        return mClient.call(*this, pValue);
    }

    void setHRcpHandler(std::function<bfc::Buffer(const bfc::BufferView&)>&& pHandler)
    {
        mNode->rcpHandler = std::move(pHandler);
    }

private:
    Property(Client& pClient, std::shared_ptr<Node> pNode)
        : mClient(pClient)
        , mNode(pNode)
    {}

    std::shared_ptr<Node>& node()
    {
        return mNode;
    }

    uint64_t uuid()
    {
        return mNode->uuid;
    }

    friend class Client;
    Client& mClient;
    std::shared_ptr<Node> mNode;
};

} // propertytree

#endif // __PROPERTY_HPP__