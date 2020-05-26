#ifndef __PROPERTY_HPP__
#define __PROPERTY_HPP__

#include <bfc/EpollReactor.hpp>

#include <propertytree/Node.hpp>
#include <propertytree/Client.hpp>

namespace propertytree
{

class Property
{
public:
    Property(Client& pClient, std::shared_ptr<Node> pNode)
        : mClient(pClient)
        , mNode(pNode)
    {}

    template <typename T>
    Property& operator=(const T& pOther)
    {
        auto& node = *mNode;
        if (sizeof(pOther) == node.data.size())
        {
            new (node.data.data()) T(pOther);
        }
        else
        {
            auto raw = new T(pOther);
            node.data = bfc::Buffer((std::byte*)raw, sizeof(T)); 
        }

        mClient.commit(*this);
        return *this;
    }

    template <typename T>
    std::optional<T> value()
    {
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

    operator bool() const
    {
        return mNode ? true : false;
    }

private:
    std::shared_ptr<Node> node()
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