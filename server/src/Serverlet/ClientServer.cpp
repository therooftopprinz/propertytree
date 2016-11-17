#include <sstream>
#include <iostream>
#include <algorithm>
#include <thread>
#include "ClientServer.hpp"
#include <server/src/Utils.hpp>

namespace ptree
{
namespace server
{

ClientServerMonitor::ClientServerMonitor() :
    log(logger::Logger("ClientServerMonitor"))
{
}

ClientServerMonitor::~ClientServerMonitor()
{
}

void ClientServerMonitor::addClientServer(ClientServerPtr clientServer)
{
    std::lock_guard<std::mutex> guard(clientServersMutex);
    log << logger::DEBUG << "adding client server to monitor " << (void*)clientServer.get();
    clientServers.push_back(clientServer);
}

void ClientServerMonitor::removeClientServer(ClientServerPtr clientServer)
{
    std::lock_guard<std::mutex> guard(clientServersMutex);
    log << logger::DEBUG << "removing client server to monitor " << (void*)clientServer.get();
    auto it = std::find(clientServers.begin(), clientServers.end(), clientServer);
    if (it != clientServers.end())
    {  
        clientServers.erase(it);
        log << logger::DEBUG << "erased!" << (void*)clientServer.get();
    }
    else
    {
        log << logger::ERROR << "not found!" << (void*)clientServer.get();
    }
}

void ClientServerMonitor::notifyCreation(uint32_t uuid, protocol::PropertyType type,
    std::string path)
{
    std::lock_guard<std::mutex> guard(clientServersMutex);
    for (auto& clientServer : clientServers)
    {
        clientServer->notifyCreation(uuid, type, path);
    }
}

void ClientServerMonitor::notifyDeletion(uint32_t uuid)
{
    std::lock_guard<std::mutex> guard(clientServersMutex);
    for (auto& clientServer : clientServers)
    {
        clientServer->notifyDeletion(uuid);
    }
}

void ClientServer::setup()
{
    if (isSetup)
        return;
    log << logger::DEBUG << "ClientServer::setup(): Setup begin...";
    isSetup = true;
    monitor->addClientServer(shared_from_this());
    std::function<void()> incoming = std::bind(&ClientServer::handleIncoming, this);
    std::function<void()> outgoing = std::bind(&ClientServer::handleOutgoing, this);
    killHandleIncoming = false;
    killHandleOutgoing = false;
    log << logger::DEBUG << "Creating incomingThread.";
    std::thread incomingThread(incoming);
    log << logger::DEBUG << "Creating outgoingThread";
    std::thread outgoingThread(outgoing);   
    incomingThread.detach();
    outgoingThread.detach();
    log << logger::DEBUG << "Created threads detached.";
}

void ClientServer::teardown()
{
    if (!isSetup)
        return;

    log << logger::DEBUG << "teardown: teardown begin...";
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(100ms); // wait setup to finish

    killHandleIncoming = true;
    killHandleOutgoing = true;

    log << logger::DEBUG << "teardown: removing from monitor";
    monitor->removeClientServer(shared_from_this());

    log << logger::DEBUG << "teardown: waiting thread to stop...";
    log << logger::DEBUG << "teardown: handleIncoming " << handleIncomingIsRunning;
    log << logger::DEBUG << "teardown: handleOutgoing " << handleOutgoingIsRunning;
    log << logger::DEBUG << "teardown: prossesing " << processMessageRunning;
    while (handleIncomingIsRunning || handleOutgoingIsRunning || processMessageRunning);
    log << logger::DEBUG << "ClientServer::teardown(): Teardown complete.";
    isSetup = false;
}

void ClientServer::processMessage(protocol::MessageHeaderPtr header, BufferPtr message)
{
    processMessageRunning++;
    log << logger::DEBUG << "ClientServer::processMessage()";
    auto type = header->type;
    std::lock_guard<std::mutex> guard(sendLock);
    messageHandlerFactory.get(type, *this, *endpoint, *ptree, *monitor)->handle(header, message);
    processMessageRunning--;
}

void ClientServer::handleIncoming()
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

void ClientServer::notifyValueUpdate(core::ValuePtr value)
{
    std::lock_guard<std::mutex> guard(valueUpdateNotificationMutex);
    valueUpdateNotification.push_back(value);
}

void ClientServer::notifyCreation(uint32_t uuid, protocol::PropertyType type,
    std::string path)
{
    std::lock_guard<std::mutex> guard(metaUpdateNotificationMutex);
    log << logger::DEBUG << "notifyCreation for: " << uuid;
    auto it = metaUpdateNotification.find(uuid);
    if (metaUpdateNotification.find(uuid) == metaUpdateNotification.end())
    {
        log << logger::DEBUG << "notification queued!";
        ActionTypeAndPath a;
        a.utype = ActionTypeAndPath::UpdateType::CREATE_OBJECT;
        a.ptype = type;
        a.path = path;
        metaUpdateNotification[uuid] = a;
    }
    else if (metaUpdateNotification[uuid].utype ==
        ActionTypeAndPath::UpdateType::DELETE_OBJECT)
    {
        log << logger::DEBUG << "Delete was already queued! Canceling.";
        metaUpdateNotification.erase(it);
    }
    else
    {
        log << logger::ERROR << "Error queued again!";
    }
}

void ClientServer::notifyDeletion(uint32_t uuid)
{
    // log << logger::DEBUG << "notifyDeletion for:" << uuid;
    // std::lock_guard<std::mutex> guard(metaUpdateNotificationMutex);
    // auto it = metaUpdateNotification.find(uuid);
    // if (metaUpdateNotification.find(uuid) == metaUpdateNotification.end())
    // {
    //     log << logger::DEBUG << "notifaction queued!";
    //     ActionTypeAndPath a;
    //     a.utype = protocol::MetaUpdateNotification::UpdateType::DELETE_OBJECT;
    //     metaUpdateNotification[uuid] = a;
    // }
    // else if (metaUpdateNotification[uuid].utype ==
    //     protocol::MetaUpdateNotification::UpdateType::CREATE_OBJECT)
    // {
    //     log << logger::DEBUG << "Create was already queued! Canceling.";
    //     metaUpdateNotification.erase(it);
    // }
    // else
    // {
    //     log << logger::ERROR << "Error queued again!";
    // }
}

void ClientServer::setUpdateInterval(uint32_t interval)
{
    updateInterval = interval;
}

void ClientServer::clientSigned()
{
    log << logger::DEBUG << "ClientServer state signed in.";
    isSignin = true;
}

void ClientServer::handleOutgoing()
{
    handleOutgoingIsRunning = true;
    while (!killHandleOutgoing)
    {
        if (!isSignin)
        {
            continue;
        }
        if (metaUpdateNotification.size())
        {
            log << logger::DEBUG << "Meta Notifaction available!";
            std::lock_guard<std::mutex> updatenotifGuard(metaUpdateNotificationMutex);
            std::lock_guard<std::mutex> sendGuard(sendLock);

            protocol::MetaUpdateNotification metaUpdateNotif;
            for(const auto& i : metaUpdateNotification)
            {
                if (i.second.utype == ActionTypeAndPath::UpdateType::CREATE_OBJECT)
                {
                    metaUpdateNotif.creations->push_back(protocol::MetaCreate(i.first, i.second.ptype, i.second.path));
                }
                else
                {
                    metaUpdateNotif.deletions->push_back(protocol::MetaDelete(i.first));
                }
            }

            Buffer notifheader =
                MessageHandler::createHeader(protocol::MessageType::MetaUpdateNotification, metaUpdateNotif.size(),
                    static_cast<uint32_t>(-1));
            endpoint->send(notifheader.data(), notifheader.size());

            Buffer enbuff(metaUpdateNotif.size());
            protocol::BufferView enbuffv(enbuff);
            protocol::Encoder en(enbuffv);
            metaUpdateNotif >> en;
            endpoint->send(enbuff.data(), enbuff.size());

            log << logger::DEBUG << "MetaUpdateNotification sent!";
            metaUpdateNotification.clear();
        }

        if (valueUpdateNotification.size())
        {
            log << logger::DEBUG << "Property Update Notifaction available!";
            std::lock_guard<std::mutex> updatenotifGuard(valueUpdateNotificationMutex);
            std::lock_guard<std::mutex> sendGuard(sendLock);

            // MessagePropertyUpdateNotificationSender updateNotif(valueUpdateNotification, endpoint);
            // updateNotif.send();

            log << logger::DEBUG << "PropertyUpdateNotification sent!";
            valueUpdateNotification.clear();
        }

        using namespace std::chrono_literals;
        /** TODO: This wait should be the from sign in message update interval **/
        std::this_thread::sleep_for(1us); // wait update rate
    }
    log << logger::DEBUG << "handleOutgoing: exiting..";
    handleOutgoingIsRunning = false;
}


} // namespace server
} // namespace ptree
