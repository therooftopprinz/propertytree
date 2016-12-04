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
    std::list<protocol::SigninRequest::FeatureFlag> features;
    features.push_back(protocol::SigninRequest::FeatureFlag::ENABLE_METAUPDATE);
    sendSignIn(300, features);
}

void PTreeClient::sendSignIn(int refreshRate, const std::list<protocol::SigninRequest::FeatureFlag> features)
{
    std::lock_guard<std::mutex> guard(sendLock);
    protocol::SigninRequest signIn;
    signIn.version = 1;
    signIn.refreshRate = refreshRate;
    for (const auto& i : features)
    {
        signIn.setFeature(i);
    }
    auto tid = transactionIdGenerator.get();
    messageSender(tid, protocol::MessageType::SigninRequest, signIn);
    addTransactionCV(tid);
    if (waitTransactionCV(tid))
    {
        log << logger::DEBUG << "SIGNIN OK";
    }
    else
    {
        log << logger::ERROR << "SIGNIN OK TIMEOUT";
    }
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

void PTreeClient::addTransactionCV(uint32_t transactionId)
{
    std::lock_guard<std::mutex> guard(transactionIdCVLock);
    transactionIdCV[transactionId] = std::make_shared<TransactionCV>();
}

void PTreeClient::notifyTransactionCV(uint32_t transactionId)
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