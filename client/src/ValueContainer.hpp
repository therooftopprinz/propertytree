#ifndef CLIENT_VALUECONTAINER_HPP_
#define CLIENT_VALUECONTAINER_HPP_

#include <cassert>
#include <map>
#include <vector>
#include <set>
#include <string>
#include <mutex>
#include <thread>
#include <memory>

#include <common/src/Logger.hpp>

namespace ptree
{
namespace client
{

class PTreeClient;
typedef std::shared_ptr<PTreeClient> PTreeClientPtr;

class ValueContainer;
typedef std::shared_ptr<ValueContainer> ValueContainerPtr;

typedef std::vector<uint8_t> Buffer;
typedef std::shared_ptr<Buffer> BufferPtr;

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

    Buffer getCopy();

    void addWatcher(std::shared_ptr<IValueWatcher> watcher);
    void removeWatcher(std::shared_ptr<IValueWatcher> watcher);

    ValueContainer(PTreeClientPtr ptc, Buffer &value);
    ValueContainer(PTreeClientPtr ptc, Buffer &&value);

private:
    bool isAutoUpdate();
    void setAutoUpdate(bool autoUpdate);
    void updateValue(Buffer&& value, bool triggerHandler);

    std::set<std::shared_ptr<IValueWatcher>> watchers;
    std::mutex watcherMutex;

    bool autoUpdate;
    std::weak_ptr<PTreeClient> ptreeClient;
    Buffer value;
    std::mutex valueMutex;
    logger::Logger log;

    friend class PTreeClient;
};

}
}

#endif // CLIENT_VALUECONTAINER_HPP_
