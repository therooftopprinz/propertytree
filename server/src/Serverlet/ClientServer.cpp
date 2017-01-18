#include <sstream>
#include <iostream>
#include <algorithm>
#include <thread>
#include "ClientServer.hpp"
#include <common/src/Utils.hpp>

namespace ptree
{
namespace server
{

ClientNotifier::ClientNotifier() :
    log(logger::Logger("ClientNotifier"))
{
}

ClientNotifier::~ClientNotifier()
{
}

void ClientNotifier::addClientServer(uint64_t clienServerId, IPTreeOutgoing& clientNotifier)
{
    std::lock_guard<std::mutex> guard(clientNotifierMutex);
    log << logger::DEBUG << "adding client server to notifier, csid= " << clienServerId;
    clientNotifiers.emplace(clienServerId, clientNotifier);
}

void ClientNotifier::removeClientServer(uint64_t clienServerId)
{
    std::lock_guard<std::mutex> guard(clientNotifierMutex);
    log << logger::DEBUG << "removing client server to notifier, csid= " << clienServerId;
    auto it = clientNotifiers.find(clienServerId);
    if (it != clientNotifiers.end())
    {
        clientNotifiers.erase(it);
        log << logger::DEBUG << "erased! csid=" << clienServerId;
    }
    else
    {
        log << logger::ERROR << "not found! csid=" << clienServerId;
    }
}

void ClientNotifier::notifyCreation(uint32_t uuid, protocol::PropertyType type, std::string path)
{
    std::lock_guard<std::mutex> guard(clientNotifierMutex);
    for (auto& clientNotifier : clientNotifiers)
    {
        clientNotifier.second.get().notifyCreation(uuid, type, path);
    }
}

void ClientNotifier::notifyDeletion(uint32_t uuid)
{
    std::lock_guard<std::mutex> guard(clientNotifierMutex);
    for (auto& clientNotifier : clientNotifiers)
    {
        clientNotifier.second.get().notifyDeletion(uuid);
    }
}

void ClientNotifier::notifyRpcResponse(uint64_t clientServerId, uint32_t transactionId, Buffer&& returnValue)
{
    std::lock_guard<std::mutex> guard(clientNotifierMutex);
    auto it = clientNotifiers.find(clientServerId);
    if (it != clientNotifiers.end())
    {
        it->second.get().notifyRpcResponse(transactionId, std::move(returnValue));;
    }
}

void ClientServer::setup()
{
    if (isSetup)
        return;
    log << logger::DEBUG << "ClientServer::setup(): Setup begin...";
    isSetup = true;
    notifier->addClientServer(clienServerId, outgoing);
    std::function<void()> incoming = std::bind(&ClientServer::handleIncoming, this);
    killHandleIncoming = false;
    log << logger::DEBUG << "Creating incomingThread.";
    std::thread incomingThread(incoming); 
    incomingThread.detach();
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

    log << logger::DEBUG << "teardown: removing from notifier";
    notifier->removeClientServer(clienServerId);

    log << logger::DEBUG << "teardown: waiting thread to stop...";
    log << logger::DEBUG << "teardown: handleIncoming " << handleIncomingIsRunning;
    log << logger::DEBUG << "teardown: prossesing " << processMessageRunning;
    while (handleIncomingIsRunning || processMessageRunning);
    log << logger::DEBUG << "ClientServer::teardown(): Teardown complete.";
    isSetup = false;
}

void ClientServer::processMessage(protocol::MessageHeaderPtr header, BufferPtr message)
{
    processMessageRunning++;
    log << logger::DEBUG << "ClientServer::processMessage()";
    auto type = header->type;
    std::lock_guard<std::mutex> guard(sendLock);
    auto lval_this = std::dynamic_pointer_cast<ClientServer>(shared_from_this());
    /** TODO: Remove after investigation is done **/
    // if (type==protocol::MessageType::SubscribePropertyUpdateRequest)
    //     std::make_unique<SubscribePropertyUpdateRequestMessageHandler>(lval_this, *endpoint.get(), *ptree.get(), *notifier.get())->handle(header, message);
    // else
        IPTreeOutgoingPtr outgoingAliased(shared_from_this(), &outgoing);
        MessageHandlerFactory::get(clienServerId, type, config, outgoingAliased, ptree, notifier)->handle(header, message);

    processMessageRunning--;
}

void ClientServer::handleIncoming()
{
    handleIncomingIsRunning = true;
    const uint8_t HEADER_SIZE = sizeof(protocol::MessageHeader);
    log << logger::DEBUG << "handleIncoming: Spawned.";
    incomingState = EIncomingState::WAIT_FOR_HEADER_EMPTY;

    using namespace std::chrono_literals;

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

                if (incomingState != EIncomingState::WAIT_FOR_HEADER_EMPTY)
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

                std::this_thread::sleep_for(1ms);

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

                if (incomingState != EIncomingState::WAIT_FOR_MESSAGE_EMPTY)
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

                std::this_thread::sleep_for(1ms);

            }
        }
    }

    // using namespace std::chrono_literals;
    // std::this_thread::sleep_for(5s);

    log << logger::DEBUG << "handleIncoming: exiting.";
    handleIncomingIsRunning = false;
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

} // namespace server
} // namespace ptree
