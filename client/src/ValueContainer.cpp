#include "ValueContainer.hpp"

namespace ptree
{
namespace client
{

ValueContainer::ValueContainer(protocol::Uuid uuid, Buffer &value, bool ownership) :
    uuid(uuid), autoUpdate(false), ownership(ownership), value(value), log("ValueContainer")
{
}
ValueContainer::ValueContainer(protocol::Uuid uuid, Buffer &&value, bool ownership) :
    uuid(uuid), autoUpdate(false), ownership(ownership), value(std::move(value)), log("ValueContainer")
{
}

bool ValueContainer::isOwned()
{
    return ownership;
}

protocol::Uuid ValueContainer::getUuid()
{
    return uuid;
}

Buffer& ValueContainer::get()
{
    return value;
}

Buffer ValueContainer::getCopy()
{
    std::lock_guard<std::mutex> lockValue(valueMutex);
    return value;
}

bool ValueContainer::isAutoUpdate()
{
    return autoUpdate;
}

void ValueContainer::setAutoUpdate(bool b)
{
    autoUpdate = b;
}

void ValueContainer::updateValue(Buffer&& value, bool triggerHandler)
{
    {
        std::lock_guard<std::mutex> lockValue(valueMutex);
        this->value = std::move(value);
    }

    if (!triggerHandler)
    {
        return;
    }

    std::lock_guard<std::mutex> lockWatcher(watcherMutex);
    for (auto& i : watchers)
    {
        auto fn = std::bind(&IValueWatcher::handle, i, shared_from_this());
        std::thread t(fn);
        t.detach();
    }
}

void ValueContainer::addWatcher(std::shared_ptr<IValueWatcher> watcher)
{
    std::lock_guard<std::mutex> lock(watcherMutex);
    auto i = watchers.find(watcher);
    if (i==watchers.end())
    {
        watchers.insert(watcher);
    }
}

void ValueContainer::removeWatcher(std::shared_ptr<IValueWatcher> watcher)
{
    std::lock_guard<std::mutex> lock(watcherMutex);
    auto i = watchers.find(watcher);
    if (i!=watchers.end())
    {
        watchers.erase(watcher);
    }
}



}
}