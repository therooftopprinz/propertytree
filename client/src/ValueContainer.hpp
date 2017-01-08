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

namespace ptree
{
namespace client
{

struct IValueWatcher
{
    virtual void handle(ValueContainerPtr vc) = 0;
};

class ValueContainer : public std::enable_shared_from_this<ValueContainer>
{
public:
    ValueContainer() = delete;

    /** TODO: on destruction if meta uuid is not watched delete meta. **/

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

    bool isOwned();
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

    void addWatcher(std::shared_ptr<IValueWatcher> watcher);
    void removeWatcher(std::shared_ptr<IValueWatcher> watcher);

    protocol::Uuid getUuid();

    void operator = (ValueContainer&) = delete;

    ValueContainer(protocol::Uuid uuid, Buffer &value, bool owned);
    ValueContainer(protocol::Uuid uuid, Buffer &&value, bool owned);

private:
    bool isAutoUpdate();
    void setAutoUpdate(bool autoUpdate);
    void updateValue(Buffer&& value, bool triggerHandler);

    /** TODO: use std::function **/
    std::set<std::shared_ptr<IValueWatcher>> watchers;
    std::mutex watcherMutex;
    protocol::Uuid uuid;
    bool autoUpdate;
    bool owned;
    Buffer value;
    std::mutex valueMutex;
    logger::Logger log;

    friend class PTreeClient;
};

}
}

#endif // CLIENT_VALUECONTAINER_HPP_
