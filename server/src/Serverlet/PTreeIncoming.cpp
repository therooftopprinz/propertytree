#include "PTreeIncoming.hpp"
namespace ptree
{
namespace server
{

PTreeIncoming::PTreeIncoming(uint64_t clientServerId,
    ClientServerConfig& config, IEndPointPtr& endpoint, core::PTreePtr& ptree, IClientNotifierPtr& notifier):
        clientServerId(clientServerId), config(config), endpoint(endpoint), ptree(ptree),
        notifier(notifier), log("PTreeIncoming")
{
    log << logger::DEBUG << "construct";
}

PTreeIncoming::~PTreeIncoming()
{
    log << logger::DEBUG << "destruct";
    killHandleIncoming = true;
    log << logger::DEBUG << "teardown: waiting thread to stop...";
    log << logger::DEBUG << "teardown: handleIncoming " << handleIncomingIsRunning;
    while (handleIncomingIsRunning);
    log << logger::DEBUG << "Teardown complete.";
}

void PTreeIncoming::init(IPTreeOutgoingWkPtr o)
{
    outgoing = o;
    std::function<void()> incoming = std::bind(&PTreeIncoming::handleIncoming, this);
    killHandleIncoming = false;
    log << logger::DEBUG << "Creating incomingThread.";
    std::thread incomingThread(incoming); 
    incomingThread.detach();
    log << logger::DEBUG << "Created threads detached.";
    log << logger::DEBUG << "Setup complete.";
}

void PTreeIncoming::processMessage(protocol::MessageHeader& header, Buffer& message)
{
    log << logger::DEBUG << "processMessage()";
    auto type = header.type;
    auto outgoingShared = outgoing.lock();
    MessageHandlerFactory::get(clientServerId, type, config, outgoingShared, ptree, notifier)->handle(header, message);
}

void PTreeIncoming::handleIncoming()
{
    handleIncomingIsRunning = true;
    log << logger::DEBUG << "handleIncoming: Spawned.";
    enum class EIncomingState
    {
        EMPTY = 0,
        WAIT_HEAD,
        WAIT_BODY
    };

    EIncomingState incomingState = EIncomingState::EMPTY;
    protocol::MessageHeader header;
    std::vector<uint8_t> data;

    uint8_t* cursor = (uint8_t*)&header;
    uint32_t size = sizeof(protocol::MessageHeader);
    uint32_t remainingSize = sizeof(protocol::MessageHeader);
    uint8_t retryCount = 0;

    using namespace std::chrono_literals;

    while (!killHandleIncoming)
    {
        log << logger::ERROR << "STATE:" << static_cast<uint32_t>(incomingState);
        size_t receiveSize = endpoint->receive(cursor, size);

        if (incomingState == EIncomingState::EMPTY && receiveSize == 0)
        {
            continue;
        }
        else if (incomingState != EIncomingState::EMPTY && receiveSize == 0)
        {
            if (++retryCount > 3)
            {
                /**TODO: send receive failure to client**/
                log << logger::ERROR << "RECEIVE FAILED!!";
                incomingState = EIncomingState::EMPTY;
                cursor = (uint8_t*)&header;
                size = sizeof(protocol::MessageHeader);
                remainingSize = size;
                continue;
            }
        }
        else
        {
            retryCount = 0;
        }

        switch(incomingState)
        {
        case EIncomingState::EMPTY:
        case EIncomingState::WAIT_HEAD:
            log << logger::DEBUG << "receiving: " << receiveSize << " for " << remainingSize;
            incomingState = EIncomingState::WAIT_HEAD;
            remainingSize -= receiveSize;
            cursor += receiveSize;
            if (remainingSize == 0)
            {
                log << logger::DEBUG << "HEADER RECEIVED!! content size: " << header.size;
                if (header.size >= uint32_t(1024*1024))
                {
                    log << logger::ERROR << "INVALID CONTENT SIZE!!";
                    incomingState = EIncomingState::EMPTY;
                    cursor = (uint8_t*)&header;
                    size = sizeof(protocol::MessageHeader);
                    remainingSize = size;
                    continue;
                }
                incomingState = EIncomingState::WAIT_BODY;
                log << logger::DEBUG << "allocating=" << header.size << "bytes";
                data.resize(header.size);
                cursor = data.data();
                size = header.size - sizeof(protocol::MessageHeader);
                remainingSize = size;
            }
            break;
        case EIncomingState::WAIT_BODY:
            remainingSize -= receiveSize;
            cursor += receiveSize;
            if (remainingSize == 0)
            {
                log << logger::DEBUG << "handleIncoming: Message complete.";
                processMessage(header, data);
                incomingState = EIncomingState::EMPTY;
                cursor = (uint8_t*)&header;
                size = sizeof(protocol::MessageHeader);
                remainingSize = size;
            }
            break;
        }
    }

    log << logger::DEBUG << "handleIncoming: exiting.";
    handleIncomingIsRunning = false;
}

} // namespace server
} // namespace ptree
