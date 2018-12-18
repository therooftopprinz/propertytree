#include "PTreeIncoming.hpp"
namespace ptree
{
namespace server
{

PTreeIncoming::PTreeIncoming(uint64_t clientServerId,
    ClientServerConfig& config, IEndPoint& endpoint, IPTreeOutgoingPtr outgoing,
    core::PTreePtr& ptree, IPTreeServer& notifier)
        : log("PTreeIncoming")
        , clientServerId(clientServerId), config(config), endpoint(endpoint), outgoingWkPtr(outgoing)
        , createRequestMessageHandler(*ptree, notifier)
        , deleteRequestMessageHandler(*outgoing, *ptree, notifier)
        , getSpecificMetaRequestMessageHandler(*outgoing, *ptree)
        , getValueRequestMessageHandler(*outgoing, *ptree)
        , handleRpcResponseMessageHandler(notifier)
        , rpcRequestMessageHandler(clientServerId, *ptree)
        , setValueIndicationMessageHandler(*ptree)
        , signinRequestMessageHandler(*outgoing, config, *ptree, notifier)
        , subscribePropertyUpdateRequestMessageHandler(clientServerId, *ptree, notifier)
        , unsubscribePropertyUpdateRequestMessageHandler(clientServerId, *outgoing, *ptree)
        , incomingThread(std::bind(&PTreeIncoming::handleIncoming, this))
{
    log << logger::DEBUG << "construct";
}

PTreeIncoming::~PTreeIncoming()
{
    log << logger::DEBUG << "destruct";
    killHandleIncoming = true;
    log << logger::DEBUG << "teardown: waiting thread to stop...";
    log << logger::DEBUG << "teardown: handleIncoming " << handleIncomingIsRunning;
    incomingThread.join();
    log << logger::DEBUG << "Teardown complete.";
}

void PTreeIncoming::init(IPTreeOutgoingWkPtr o)
{
    // outgoingWkPtr = o;
    // std::function<void()> incoming = std::bind(&PTreeIncoming::handleIncoming, this);
    // killHandleIncoming = false;
    // log << logger::DEBUG << "Creating incomingThread.";
    // incomingThread = std::thread(incoming);
    // log << logger::DEBUG << "Created threads detached.";
    // log << logger::DEBUG << "Setup complete.";
}

void PTreeIncoming::processMessage(protocol::MessageHeader& header, Buffer& message)
{
    auto type = header.type;
    log << logger::DEBUG << "processMessage(" << uint32_t(type) << ", "
        << header.size << ", " << header.transactionId<< "): ";
    utils::printRaw(message.data(), message.size());
    auto outgoingShared = outgoingWkPtr.lock();

    using Enum = uint8_t;
    switch (uint8_t(type))
    {
        case (Enum) protocol::MessageType::SigninRequest:
            signinRequestMessageHandler.handle(header, message);
            break;
        case (Enum) protocol::MessageType::CreateRequest:
            createRequestMessageHandler.handle(outgoingShared, header, message);
            break;
        case (Enum) protocol::MessageType::DeleteRequest:
            deleteRequestMessageHandler.handle(header, message);
            break;
        case (Enum) protocol::MessageType::SetValueIndication:
            setValueIndicationMessageHandler.handle(header, message);
            break;
        case (Enum) protocol::MessageType::SubscribePropertyUpdateRequest:
            subscribePropertyUpdateRequestMessageHandler.handle(outgoingShared, header, message);
            break;
        case (Enum) protocol::MessageType::UnsubscribePropertyUpdateRequest:
            unsubscribePropertyUpdateRequestMessageHandler.handle(header, message);
            break;
        case (Enum) protocol::MessageType::GetValueRequest:
            getValueRequestMessageHandler.handle(header, message);
            break;
        case (Enum) protocol::MessageType::RpcRequest:
            rpcRequestMessageHandler.handle(header, message);
            break;
        case (Enum) protocol::MessageType::HandleRpcResponse:
            handleRpcResponseMessageHandler.handle(header, message);
            break;
        case (Enum) protocol::MessageType::GetSpecificMetaRequest:
            getSpecificMetaRequestMessageHandler.handle(header, message);
            break;
    }
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
    log << logger::DEBUG << "STATE: EMPTY";
    while (!killHandleIncoming)
    {
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
                log << logger::ERROR << "STATE: EMPTY";
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
                size = header.size - sizeof(protocol::MessageHeader);
                data.resize(size);
                cursor = data.data();
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

} // namespace server
} // namespace ptree
