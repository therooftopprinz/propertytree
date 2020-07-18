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
    Property() = default;

    Property& operator=(const Property& pOther)
    {
        mClient = pOther.mClient;
        mNode = pOther.mNode;
        return *this;
    }

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
            node.data.resize(sizeof(T));
            new (node.data.data()) T(pOther);
        }

        mClient->commit(*this);
        return *this;
    }

    void set(std::vector<uint8_t>&& pValue)
    {
        std::unique_lock<std::mutex> lg(mNode->dataMutex);
        auto& node = *mNode;

        node.data = std::move(pValue);

        mClient->commit(*this);
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

    std::vector<uint8_t> raw()
    {
        std::unique_lock<std::mutex> lg(mNode->dataMutex);
        return mNode->data;
    }

    void fetch()
    {
        mClient->fetch(*this);
    }

    Property create(const std::string& pName)
    {
        return mClient->create(*this, pName);
    }

    Property createOrGet(const std::string& pName)
    {
        auto rv = get(pName);
        if (!rv)
        {
            rv = create(pName);
        }
        return rv;
    }

    Property get(const std::string& pName)
    {
        return mClient->get(*this, pName, false);
    }

    operator bool() const
    {
        return mNode ? true : false;
    }

    void subscribe()
    {
        mClient->subscribe(*this);
    }

    void unsubscribe()
    {
        mClient->unsubscribe(*this);
    }

    bool destroy()
    {
        return mClient->destroy(*this);
    }

    void loadChildren(bool pRecursive = false)
    {
        mClient->get(*this, ".", pRecursive);
    }

    std::vector<std::pair<std::string, Property>> children()
    {
        std::vector<std::pair<std::string, Property>> rv;
        std::unique_lock<std::mutex> lg(mNode->childrenMutex);
        for (auto& i : mNode->children)
        {
            rv.emplace_back(i.first, Property(*mClient, i.second));
        }
        return rv;
    }

    size_t childrenSize()
    {
        std::unique_lock<std::mutex> lg(mNode->childrenMutex);
        return mNode->children.size();
    }

    std::vector<uint8_t> call(const bfc::BufferView& pValue)
    {
        return mClient->call(*this, pValue);
    }

    void setHRcpHandler(std::function<std::vector<uint8_t>(const bfc::BufferView&)>&& pHandler)
    {
        std::unique_lock<std::mutex> lgRpcHanlder(mNode->rcpHandlerMutex);
        mNode->rcpHandler = std::move(pHandler);
    }

    uint64_t uuid()
    {
        return mNode->uuid;
    }

    const std::string& name()
    {
        return mNode->name;
    }

    std::optional<Property> parent()
    {
        if (auto p = mNode->parent.lock())
        {
            return Property(*mClient, p);
        }
        return {};
    }

    void setUpdateHandler(std::function<void()> pHandler)
    {
        std::unique_lock<std::mutex> lgUpdateHandler(mNode->updateHandlerMutex);
        mNode->updateHandler = std::move(pHandler);
    }

private:
    Property(Client& pClient, std::shared_ptr<Node> pNode)
        : mClient(&pClient)
        , mNode(pNode)
    {}

    std::shared_ptr<Node>& node()
    {
        return mNode;
    }

    friend class Client;
    Client* mClient = nullptr;
    std::shared_ptr<Node> mNode;
};

} // propertytree

#endif // __PROPERTY_HPP__