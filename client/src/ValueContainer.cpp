#include "ValueContainer.hpp"

namespace ptree
{
namespace client
{

ValueContainer::ValueContainer(LocalPTree& ptree, protocol::Uuid uuid, std::string path, Buffer &value, bool owned) :
    IProperty(uuid, path, protocol::PropertyType::Value, owned),
    ptree(ptree), autoUpdate(false), value(value), log("ValueContainer")
{
}
ValueContainer::ValueContainer(LocalPTree& ptree, protocol::Uuid uuid, std::string path, Buffer &&value, bool owned) :
    IProperty(uuid, path, protocol::PropertyType::Value, owned),
    ptree(ptree), autoUpdate(false), value(std::move(value)), log("ValueContainer")
{
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

void ValueContainer::updateValue(bool triggerHandler)
{
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

void ValueContainer::updateValue(Buffer&& value, bool triggerHandler)
{
    {
        std::lock_guard<std::mutex> lockValue(valueMutex);
        this->value = std::move(value);
    }
    updateValue(triggerHandler);

}

void ValueContainer::updateValue(Buffer& value, bool triggerHandler)
{
    {
        std::lock_guard<std::mutex> lockValue(valueMutex);
        this->value = value;
    }
    updateValue(triggerHandler);
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

bool ValueContainer::enableAutoUpdate()
{
    if (ptree.enableAutoUpdate(getUuid()))
    {
        setAutoUpdate(true);
        return true;
    }
    return false;
}
bool ValueContainer::disableAutoUpdate()
{
    if (ptree.disableAutoUpdate(getUuid()))
    {
        setAutoUpdate(false);
        return true;
    }
    return false;
}

void ValueContainer::setValue(Buffer&& value)
{
    ptree.setValue(getUuid(), std::move(value));
}

}
}