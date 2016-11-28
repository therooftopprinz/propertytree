#include "PTreeClient.hpp"

namespace ptree
{
namespace client
{

void PTreeClient::processMessage(protocol::MessageHeaderPtr header, BufferPtr message)
{

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