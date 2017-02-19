#include "ClientIncoming.hpp"

namespace ptree
{
namespace client
{
ClientIncoming::ClientIncoming(TransactionsCV& transactionsCV, common::IEndPoint& endpoint, LocalPTree& ptree, IClientOutgoing& outgoing):
    transactionsCV(transactionsCV),
    endpoint(endpoint),
    ptree(ptree),
    outgoing(outgoing),
    log("ClientIncoming")
{
    std::function<void()> incoming = std::bind(&ClientIncoming::handleIncoming, this);
    killHandleIncoming = false;
    handleIncomingIsRunning = true;
    log << logger::DEBUG << "Creating incomingThread.";
    std::thread incomingThread(incoming);
    incomingThread.detach();
    log << logger::DEBUG << "Created threads detached.";
    log << logger::DEBUG << "Signing to server.";
}

ClientIncoming::~ClientIncoming()
{
    log << logger::DEBUG << "PTreeClient teardown begin...";
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(100ms); // wait setup to finish

    killHandleIncoming = true;

    log << logger::DEBUG << "teardown: waiting thread to stop...";
    log << logger::DEBUG << "teardown: handleIncoming " << handleIncomingIsRunning;
    while (handleIncomingIsRunning);
    log << logger::DEBUG << "ClientIncoming Teardown complete.";
}


void ClientIncoming::processMessage(protocol::MessageHeader& header, Buffer& message)
{
    log << logger::DEBUG << "processMessage()";

    auto h = MessageHandlerFactory::get(header.type, transactionsCV, ptree, outgoing);
    if (h)
    {
        h->handle(header, message);
    }
}

void ClientIncoming::handleIncoming()
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
        size_t receiveSize = endpoint.receive(cursor, remainingSize);

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
            incomingState = EIncomingState::WAIT_HEAD;
            log << logger::DEBUG << "STATE: WAIT_HEAD";
            remainingSize -= receiveSize;
            cursor += receiveSize;
            if (remainingSize == 0)
            {
                if (header.size >= uint32_t(1024*1024))
                {
                    log << logger::ERROR << "INVALID CONTENT SIZE!!";
                    incomingState = EIncomingState::EMPTY;
                    log << logger::DEBUG << "STATE: EMPTY";
                    cursor = (uint8_t*)&header;
                    size = sizeof(protocol::MessageHeader);
                    remainingSize = size;
                    continue;
                }
                incomingState = EIncomingState::WAIT_BODY;
                log << logger::DEBUG << "STATE: WAIT_BODY";
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
                processMessage(header, data);
                incomingState = EIncomingState::EMPTY;
                log << logger::DEBUG << "STATE: EMPTY";
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


}
}