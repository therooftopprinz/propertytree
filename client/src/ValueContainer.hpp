#ifndef CLIENT_VALUECONTAINER_HPP_
#define CLIENT_VALUECONTAINER_HPP_

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
#include <client/src/LocalPTree.hpp>

namespace ptree
{
namespace client
{

struct IValueWatcher
{
    virtual void handle(ValueContainerPtr vc) = 0;
};

class LocalPTree;
class ValueContainer : public IProperty, public std::enable_shared_from_this<ValueContainer>
{
public:
    ValueContainer() = delete;

    template<typename T>
    T& get()
    {
        std::lock_guard<std::mutex> lock(valueMutex);
        if (sizeof(T) != value.size())
        {
            using namespace std::chrono_literals;
            std::this_thread::sleep_for(100ms); // Wait all logs to flush
            assert(true);
        }
        return *(T*)(value.data());
    }
    Buffer& get();

    Buffer getCopy();
    template<typename T>
    T getCopy()
    {
        std::lock_guard<std::mutex> lock(valueMutex);
        if (sizeof(T) != value.size())
        {
            using namespace std::chrono_literals;
            std::this_thread::sleep_for(100ms); // Wait all logs to flush
            assert(true);
        }
        return *(T*)(value.data());
    }

    template <typename T>
    void setValue(T&& value)
    {
        if (!isOwned())
        {
            return;
        }
        if (sizeof(T) != this->value.size())
        {
            Buffer tmv(sizeof(T));
            std::memcpy(tmv.data(), &value, sizeof(T));
            updateValue(tmv, true);
            setValue(std::move(tmv));
        }
        else
        {
            *(typename std::remove_reference<T>::type *)(this->value.data()) = value;
            notifyWatchers();
            setValue(this->value);
        }
    }

    void addWatcher(std::shared_ptr<IValueWatcher> watcher);
    void removeWatcher(std::shared_ptr<IValueWatcher> watcher);
    bool enableAutoUpdate();
    bool disableAutoUpdate();

    void operator = (ValueContainer&) = delete;

    ValueContainer(LocalPTree& ptree, protocol::Uuid uuid, std::string path, Buffer &value, bool owned);
    ValueContainer(LocalPTree& ptree, protocol::Uuid uuid, std::string path, Buffer &&value, bool owned);

private:
    bool isAutoUpdate();
    void setAutoUpdate(bool autoUpdate);
    void setValue(Buffer&& value);
    void setValue(Buffer& value);

    void notifyWatchers();
    void updateValue(Buffer&& value, bool triggerHandler);
    void updateValue(Buffer& value, bool triggerHandler);
    /** TODO: use std::function **/
    LocalPTree& ptree;
    std::set<std::shared_ptr<IValueWatcher>> watchers;
    std::mutex watcherMutex;
    bool autoUpdate;
    Buffer value;
    std::mutex valueMutex;
    logger::Logger log;

    friend class LocalPTree;
};

}
}

#endif // CLIENT_VALUECONTAINER_HPP_
