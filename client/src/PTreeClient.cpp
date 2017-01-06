#include "PTreeClient.hpp"
#include "MessageHandlerFactory.hpp"

namespace ptree
{
namespace client
{

PTreeClient::PTreeClient(common::IEndPointPtr endpoint):
    processMessageRunning(0),
    endpoint(endpoint),
    log("PTreeClient")
{
    std::function<void()> incoming = std::bind(&PTreeClient::handleIncoming, this);
    killHandleIncoming = false;
    log << logger::DEBUG << "Creating incomingThread.";
    std::thread incomingThread(incoming);
    incomingThread.detach();
    log << logger::DEBUG << "Created threads detached.";
    log << logger::DEBUG << "Signing to server.";
}

void PTreeClient::addMeta(protocol::Uuid uuid, std::string path, protocol::PropertyType type)
{
    std::lock_guard<std::mutex> lock(uuidMetaMapMutex);
    uuidMetaMap[uuid] = PTreeMeta(path,type);  
    pathUuidMap[path] = uuid;
}

void PTreeClient::removeMeta(protocol::Uuid uuid)
{
    std::lock_guard<std::mutex> lock(uuidMetaMapMutex);
    auto i = uuidMetaMap.find(uuid);
    if (i == uuidMetaMap.end())
    {
       return;
    }
    auto j = pathUuidMap.find(i->second.path);
    uuidMetaMap.erase(i);
    pathUuidMap.erase(j);
}

protocol::Uuid PTreeClient::getUuid(std::string path)
{
    std::lock_guard<std::mutex> lock(uuidMetaMapMutex);
    auto i = pathUuidMap.find(path);
    if (i == pathUuidMap.end())
    {
        return static_cast<protocol::Uuid>(-1);
    }
    return i->second;
}

std::string PTreeClient::getPath(protocol::Uuid uuid)
{
    std::lock_guard<std::mutex> lock(uuidMetaMapMutex);
    auto i = uuidMetaMap.find(uuid);
    if (i == uuidMetaMap.end())
    {
        return std::string();
    }
    return i->second.path;
}
 
PTreeClient::PTreeMeta PTreeClient::getMeta(protocol::Uuid uuid)
{
    std::lock_guard<std::mutex> lock(uuidMetaMapMutex);
    auto i = uuidMetaMap.find(uuid);
    if (i == uuidMetaMap.end())
    {
        return PTreeMeta();
    }
    return i->second;
}

PTreeClient::~PTreeClient()
{
    log << logger::DEBUG << "PTreeClient teardown begin...";
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(100ms); // wait setup to finish

    killHandleIncoming = true;

    log << logger::DEBUG << "teardown: waiting thread to stop...";
    log << logger::DEBUG << "teardown: handleIncoming " << handleIncomingIsRunning;
    log << logger::DEBUG << "teardown: prossesing " << processMessageRunning;
    while (handleIncomingIsRunning || processMessageRunning);
    log << logger::DEBUG << "PTreeClient Teardown complete.";
}

void PTreeClient::signIn()
{
    std::list<protocol_x::SigninRequest::FeatureFlag> features;
    features.push_back(protocol_x::SigninRequest::FeatureFlag::ENABLE_METAUPDATE);
    sendSignIn(300, features);
}

void PTreeClient::sendSignIn(int refreshRate, const std::list<protocol_x::SigninRequest::FeatureFlag> features)
{
    protocol_x::SigninRequest signIn;
    signIn.version = 1;
    signIn.refreshRate = refreshRate;
    for (const auto& i : features)
    {
        signIn.setFeature(i);
    }
    auto tid = getTransactionId();
    messageSender(tid, protocol::MessageType::SigninRequest, signIn);
    auto tcv = addTransactionCV(tid);
    if (waitTransactionCV(tid))
    {
        log << logger::DEBUG << "signin response received.";

        protocol_x::SigninResponse response;
        response.unpackFrom(tcv->value);
    }
    else
    {
        log << logger::ERROR << "SIGNIN TIMEOUT";
    }
}

ValueContainerPtr PTreeClient::getLocalValue(protocol::Uuid uuid)
{
    std::lock_guard<std::mutex> lock(valuesMutex);
    auto i = values.find(uuid);
    if (i == values.end())
    {
        return ValueContainerPtr();
    }

    return i->second;
}

void PTreeClient::insertLocalValue(protocol::Uuid uuid, ValueContainerPtr& value)
{
    std::lock_guard<std::mutex> lock(valuesMutex);
    values[uuid] = value;
}



ValueContainerPtr PTreeClient::createValue(std::string path, Buffer value)
{
    protocol_x::CreateRequest request;
    request.path = path;
    request.data = value;
    request.type = protocol::PropertyType::Value;
    auto tid = getTransactionId();
    messageSender(tid, protocol::MessageType::CreateRequest, request);
    auto tcv = addTransactionCV(tid);
    if (waitTransactionCV(tid))
    {
        protocol_x::CreateResponse response;
        response.unpackFrom(tcv->value);
        if ( response.response  == protocol_x::CreateResponse::Response::OK)
        {
            log << logger::DEBUG << "VALUE CREATED WITH UUID " << response.uuid;
            auto vc = std::make_shared<ValueContainer>(response.uuid, value, true);
            insertLocalValue(response.uuid, vc);
            return vc;
        }
        else
        {
            log << logger::ERROR << "VALUE CREATE REQUEST NOT OK";
        }
    }
    else
    {
        log << logger::ERROR << "VALUE CREATE REQUEST TIMEOUT";
    }
    return ValueContainerPtr();
}

ValueContainerPtr PTreeClient::sendGetValue(protocol::Uuid uuid, ValueContainerPtr& vc)
{
    protocol_x::GetValueRequest request;
    request.uuid = uuid;
    auto tid = getTransactionId();
    messageSender(tid, protocol::MessageType::GetValueRequest, request);
    auto tcv = addTransactionCV(tid);

    if (waitTransactionCV(tid))
    {
        protocol_x::GetValueResponse response;
        response.unpackFrom(tcv->value);
        if (response.data.size())
        {
            if (!vc)
            {
                vc = std::make_shared<ValueContainer>(uuid, std::move(response.data), false);
                insertLocalValue(uuid, vc);
            }
            else
            {
                vc->updateValue(std::move(response.data), false);
            }

            return vc;
        }
        else
        {
            return ValueContainerPtr();
        }
    }
    else
    {
        log << logger::ERROR << "GET VALUE REQUEST TIMEOUT";
        return ValueContainerPtr();
    }
}

protocol::Uuid PTreeClient::fetchMetaAndAddToLocal(std::string& path)
{
    protocol_x::GetSpecificMetaRequest request;
    request.path = path;
    auto tid = getTransactionId();
    messageSender(tid, protocol::MessageType::GetSpecificMetaRequest, request);
    auto tcv = addTransactionCV(tid);
    if (waitTransactionCV(tid))
    {
        protocol_x::GetSpecificMetaResponse response;
        response.unpackFrom(tcv->value);
        log << logger::DEBUG << "UUID FOR " << path << " IS " << (uint32_t)response.meta.uuid;
        if (response.meta.uuid != static_cast<protocol::Uuid>(-1))
        {
            addMeta(response.meta.uuid, response.meta.path, response.meta.propertyType);
            return response.meta.uuid;
        }
    }
    else
    {
        log << logger::ERROR << "GET SPECIFIC META REQUEST TIMEOUT";
    }
    return static_cast<protocol::Uuid>(-1);
}

std::tuple<protocol::Uuid, protocol::PropertyType> PTreeClient::fetchMeta(std::string& path)
{
    protocol_x::GetSpecificMetaRequest request;
    request.path = path;
    auto tid = getTransactionId();
    messageSender(tid, protocol::MessageType::GetSpecificMetaRequest, request);
    auto tcv = addTransactionCV(tid);
    if (waitTransactionCV(tid))
    {
        protocol_x::GetSpecificMetaResponse response;
        response.unpackFrom(tcv->value);
        log << logger::DEBUG << "UUID FOR " << path << " IS " << (uint32_t)response.meta.uuid;
        if (response.meta.uuid != static_cast<protocol::Uuid>(-1))
        {
            return std::make_tuple(response.meta.uuid, response.meta.propertyType);
        }
    }
    else
    {
        log << logger::ERROR << "GET SPECIFIC META REQUEST TIMEOUT";
    }
    return std::make_tuple(static_cast<protocol::Uuid>(-1), static_cast<protocol::PropertyType>(-1));
}

void PTreeClient::setValue(ValueContainerPtr& vc, Buffer& data)
{
    auto uuid = vc->getUuid();
    log << logger::DEBUG << "SEND VALUE (" << uuid << ")";

    Buffer tmv = data;
    vc->updateValue(std::move(tmv), true);

    sendSetValue(vc);
}

void PTreeClient::sendSetValue(ValueContainerPtr& vc)
{
    auto uuid = vc->getUuid();
    log << logger::DEBUG << "SEND VALUE (" << uuid << ")";

    protocol_x::SetValueIndication indication;
    indication.uuid = uuid;
    indication.data = vc->get();
    auto tid = getTransactionId();
    messageSender(tid, protocol::MessageType::SetValueIndication, indication);
}


ValueContainerPtr PTreeClient::getValue(std::string path)
{
    auto uuid = getUuid(path);

    log << logger::DEBUG << "GET VALUE (" << uuid << ")" << path;

    if (uuid == static_cast<protocol::Uuid>(-1) && (uuid = fetchMetaAndAddToLocal(path)) == static_cast<protocol::Uuid>(-1))
    {
        return ValueContainerPtr();
    }

    auto vc = getLocalValue(uuid);
    if ((vc && vc->isAutoUpdate()) || (vc && vc->isOwned()))
    {
        return vc;
    }

    return sendGetValue(uuid, vc);
}

void PTreeClient::addMetaWatcher(std::shared_ptr<IMetaUpdateHandler> handler)
{
    std::lock_guard<std::mutex> lock(metaUpdateHandlersMutex);
    auto i = std::find(metaUpdateHandlers.begin(), metaUpdateHandlers.end(), handler);
    if (i == metaUpdateHandlers.end())
    {
        metaUpdateHandlers.emplace_back(handler);
    }
}

void PTreeClient::deleteMetaWatcher(std::shared_ptr<IMetaUpdateHandler> handler)
{
    std::lock_guard<std::mutex> lock(metaUpdateHandlersMutex);
    auto i = std::find(metaUpdateHandlers.begin(), metaUpdateHandlers.end(), handler);
    if (i == metaUpdateHandlers.end())
    {
        return;
    }
    metaUpdateHandlers.erase(i);
}

void PTreeClient::triggerMetaUpdateWatchersCreate(std::string& path, protocol::PropertyType propertyType)
{
    std::lock_guard<std::mutex> lock(metaUpdateHandlersMutex);
    for (auto& i : metaUpdateHandlers)
    {
        i->handleCreation(path, propertyType);
    }
}

void PTreeClient::triggerMetaUpdateWatchersDelete(protocol::Uuid uuid)
{
    std::lock_guard<std::mutex> lock(metaUpdateHandlersMutex);
    for (auto& i : metaUpdateHandlers)
    {
        i->handleDeletion(uuid);
    }
}

bool PTreeClient::createNode(std::string path)
{
    protocol_x::CreateRequest request;
    request.path = path;
    request.type = protocol::PropertyType::Node;
    auto tid = getTransactionId();
    messageSender(tid, protocol::MessageType::CreateRequest, request);
    auto tcv = addTransactionCV(tid);
    if (waitTransactionCV(tid))
    {
        protocol_x::CreateResponse response;
        response.unpackFrom(tcv->value);
        if (response.response  == protocol_x::CreateResponse::Response::OK)
        {
            log << logger::DEBUG << "NODE CREATED WITH UUID " << response.uuid;
            return true;
        }
        else
        {
            log << logger::ERROR << "NODE CREATE REQUEST NOT OK";
        }
    }
    else
    {
        log << logger::ERROR << "NODE CREATE REQUEST TIMEOUT";
    }
    return false;
}

bool PTreeClient::enableAutoUpdate(ValueContainerPtr& vc)
{
    auto uuid = vc->getUuid();
    protocol_x::SubscribePropertyUpdateRequest request;
    request.uuid = uuid;
    auto tid = getTransactionId();
    messageSender(tid, protocol::MessageType::SubscribePropertyUpdateRequest, request);
    auto tcv = addTransactionCV(tid);
    if (waitTransactionCV(tid))
    {
        protocol_x::SubscribePropertyUpdateResponse response;
        response.unpackFrom(tcv->value);
        if (response.response  == protocol_x::SubscribePropertyUpdateResponse::Response::OK)
        {
            vc->setAutoUpdate(true);
            log << logger::DEBUG << "SUBSCRIBED!! " << uuid;
            return true;
        }
        else
        {
            log << logger::ERROR << "PLEASE CHECK PATH IS CORRECT AND A VALUE.";
        }
    }
    else
    {
        log << logger::ERROR << "SUBSCRIBE REQUEST TIMEOUT";
    }
    return false;
}

bool PTreeClient::disableAutoUpdate(ValueContainerPtr& vc)
{
    auto uuid = vc->getUuid();
    protocol_x::UnsubscribePropertyUpdateRequest request;
    request.uuid = uuid;
    auto tid = getTransactionId();
    messageSender(tid, protocol::MessageType::UnsubscribePropertyUpdateRequest, request);
    auto tcv = addTransactionCV(tid);
    if (waitTransactionCV(tid))
    {
        protocol_x::UnsubscribePropertyUpdateResponse response;
        response.unpackFrom(tcv->value);
        if (response.response  == protocol_x::UnsubscribePropertyUpdateResponse::Response::OK)
        {
            vc->setAutoUpdate(false);
            log << logger::DEBUG << "UNSUBSCRIBED!! " << uuid;
            return true;
        }
        else
        {
            log << logger::ERROR << "PLEASE CHECK PATH IS CORRECT AND A VALUE.";
        }
    }
    else
    {
        log << logger::ERROR << "UNSUBSCRIBE REQUEST TIMEOUT";
    }
    return false;
}

void PTreeClient::handleUpdaNotification(protocol::Uuid uuid, Buffer&& value)
{
    log << logger::DEBUG << "Handling update for " << (uint32_t)uuid;
    std::lock_guard<std::mutex> lock(valuesMutex);
    auto i = values.find(uuid);
    if (i == values.end())
    {
        log << logger::WARNING << "Updated value not in local values. Not updating.";
        return;
    }

    i->second->updateValue(std::move(value), true);
}

void PTreeClient::processMessage(protocol::MessageHeaderPtr header, BufferPtr message)
{
    processMessageRunning++;
    log << logger::DEBUG << "processMessage()";
    auto type = header->type;
    auto lval_this = shared_from_this();
    MessageHandlerFactory::get(type, lval_this, endpoint)->handle(header, message);

    processMessageRunning--;
}

Buffer PTreeClient::createHeader(protocol::MessageType type, uint32_t payloadSize, uint32_t transactionId)
{
    Buffer header(sizeof(protocol::MessageHeader));
    protocol::MessageHeader& headerRaw = *((protocol::MessageHeader*)header.data());
    headerRaw.type = type;
    headerRaw.size = payloadSize+sizeof(protocol::MessageHeader);
    headerRaw.transactionId = transactionId;
    return header;
}

std::shared_ptr<PTreeClient::TransactionCV> PTreeClient::addTransactionCV(uint32_t transactionId)
{
    std::lock_guard<std::mutex> guard(transactionIdCVLock);
    // TODO: emplace
    transactionIdCV[transactionId] = std::make_shared<PTreeClient::TransactionCV>();
    return transactionIdCV[transactionId];
}

void PTreeClient::notifyTransactionCV(uint32_t transactionId, BufferPtr value)
{
    std::shared_ptr<TransactionCV> tcv;
    {
        std::lock_guard<std::mutex> guard(transactionIdCVLock);
        auto it = transactionIdCV.find(transactionId);
        if (it == transactionIdCV.end())
        {
            log << logger::ERROR << "transactionId not found in CV list.";
            return;
        }
        tcv = it->second;
    }

    tcv->condition = true;

    {
        std::lock_guard<std::mutex> guard(tcv->mutex);
        tcv->value = std::move(*value);
        tcv->cv.notify_all();
    }

    {
        std::lock_guard<std::mutex> guard(transactionIdCVLock);
        auto it = transactionIdCV.find(transactionId);
        if (it == transactionIdCV.end())
        {
            log << logger::ERROR << "transactionId not found in CV list.";
            return;
        }
        transactionIdCV.erase(it);
    }
}

bool PTreeClient::waitTransactionCV(uint32_t transactionId)
{
    std::shared_ptr<TransactionCV> tcv;
    {
        std::lock_guard<std::mutex> guard(transactionIdCVLock);
        auto it = transactionIdCV.find(transactionId);
        if (it == transactionIdCV.end())
        {
            log << logger::ERROR << "transactionId not found in CV list.";
            return false;
        }
        tcv = it->second;
    }

    {
        std::unique_lock<std::mutex> guard(tcv->mutex);
        using namespace std::chrono_literals;
        tcv->cv.wait_for(guard, 1s,[&tcv](){return bool(tcv->condition);});
        return tcv->condition;
    }
}

void PTreeClient::handleIncoming()
{
    handleIncomingIsRunning = true;
    const uint8_t HEADER_SIZE = sizeof(protocol::MessageHeader);
    log << logger::DEBUG << "handleIncoming: Spawned.";
    incomingState = EIncomingState::WAIT_FOR_HEADER_EMPTY;

    while (!killHandleIncoming)
    {
        // Header is a shared for the reason that I might not block processMessage
        protocol::MessageHeaderPtr header = std::make_shared<protocol::MessageHeader>();

        if (incomingState == EIncomingState::WAIT_FOR_HEADER_EMPTY)
        {
            uint8_t cursor = 0;
            uint8_t retryCount = 0;

            log << logger::DEBUG << "handleIncoming: Waiting for header.";
            while (!killHandleIncoming)
            {
                ssize_t br = endpoint->receive(header.get()+cursor, HEADER_SIZE-cursor);
                cursor += br;

                if(cursor == HEADER_SIZE)
                {
                    log << logger::DEBUG << 
                        "Header received expecting message size: " <<
                        header->size << " with type " << (uint32_t)header->type;
                    incomingState = EIncomingState::WAIT_FOR_MESSAGE_EMPTY;
                    break;
                }

                if (br == 0 && incomingState != EIncomingState::WAIT_FOR_HEADER_EMPTY)
                {
                    log << logger::DEBUG <<  "handleIncoming: Header receive timeout!";
                    retryCount++;
                }
                else if (br != 0)
                {
                    log << logger::DEBUG << "handleIncoming: Header received.";
                    incomingState = EIncomingState::WAIT_FOR_HEADER;
                }

                if (retryCount >= 3)
                {
                    log << logger::DEBUG <<
                        "handleIncoming: Header receive failed!";
                    incomingState = EIncomingState::ERROR_HEADER_TIMEOUT;
                    // TODO: ERROR HANDLING
                    break;
                }
            }
        }

        if (incomingState == EIncomingState::WAIT_FOR_MESSAGE_EMPTY)
        {
            uint8_t cursor = 0;
            uint8_t retryCount = 0;
            uint32_t expectedSize = (header->size)-HEADER_SIZE;
            BufferPtr message = std::make_shared<Buffer>(expectedSize);

            log << logger::DEBUG << "handleIncoming: Waiting for message with size: " <<
                expectedSize << " ( total " << header->size << ") with header size: " <<
                (uint32_t) HEADER_SIZE << ".";

            while (!killHandleIncoming)
            {
                ssize_t br = endpoint->receive(message->data()+cursor, expectedSize-cursor);
                cursor += br;

                if(cursor == expectedSize)
                {
                    log << logger::DEBUG << "handleIncoming: Message complete.";
                    using std::placeholders::_1;
                    incomingState = EIncomingState::WAIT_FOR_HEADER_EMPTY;
                    processMessage(header, message);
                    break;
                }

                if (br == 0 && incomingState != EIncomingState::WAIT_FOR_MESSAGE_EMPTY)
                {
                    log << logger::DEBUG <<
                        "handleIncoming: Message receive timeout!";
                    retryCount++;
                }
                else
                {
                    log << logger::DEBUG << "handleIncoming: Message received with size " << br;
                    incomingState = EIncomingState::WAIT_FOR_MESSAGE;
                }

                if (retryCount >= 3)
                {
                    log << logger::DEBUG <<
                        "handleIncoming: Message receive failed!";
                    incomingState = EIncomingState::ERROR_MESSAGE_TIMEOUT;
                    break;
                }
            }
        }
    }

    // using namespace std::chrono_literals;
    // std::this_thread::sleep_for(5s);

    log << logger::DEBUG << "handleIncoming: exiting.";
    handleIncomingIsRunning = false;
}

}
}
