#include <sstream>
#include <iostream>
#include <cassert>
#include <sstream>
#include <thread>
#include <common/src/Logger.hpp>
#include <server/src/Utils.hpp>
#include "EndPointMock.hpp"

namespace ptree
{
namespace server
{

void EndPointMock::failed(std::string msg)
{
    log << logger::ERROR << msg;
    // std::cout  << "EndPointMock failed with: " << msg << std::endl;
    EXPECT_TRUE(false);
}

ssize_t EndPointMock::send(const void *buffer, uint32_t size)
{
    log << logger::DEBUG << "Send sz:" << size;
    if (sendState == ESendState::WAITING)
    {
        sendSize = 0;
        sendState = ESendState::INCOMPLETE;
        if (size < sizeof(protocol::MessageHeader))
        {
            log << logger::ERROR << "Send called with incomplete header!";
            log << logger::ERROR << "data:";
            utils::printRaw(buffer, size);

            using namespace std::chrono_literals;
            std::this_thread::sleep_for(10ms); // wait for log to flush 
            assert(false);
        }

        protocol::MessageHeader *header = (protocol::MessageHeader*) buffer;
        expectedSendSize = header->size;
        sendBuffer.resize(expectedSendSize);
        sendBufferCursor = sendBuffer.data();
        log << logger::DEBUG << "Send expected size:" << expectedSendSize;
    }

    sendSize += size;
    if (sendSize > expectedSendSize)
    {
        log << logger::ERROR << "Expected sendSize is greater than what is sent!!";
        assert(false);
    }

    memcpy(sendBufferCursor, buffer, size);

    sendBufferCursor += size;
    if (sendSize != expectedSendSize)
    {
        log << logger::WARNING << "Send is waiting for the message to complete " <<
            sendSize << "/" << expectedSendSize;
        return size;
    }

    bool matched = false;
    uint32_t index = 0;

    log << logger::DEBUG << "Send called with:";

    buffer = sendBuffer.data();
    size = expectedSendSize;
    utils::printRaw(buffer, size);

    for (auto &i : expectations)
    {
        // if (i.cardinality == i.occurence && i.cardinality != 0)
        // {
        //     // log << logger::DEBUG << "Satified for index: " << (unsigned int) index;
        //     continue;
        // }
        
        // log << logger::DEBUG << "Matching for index: " << (unsigned int) index;

        bool dependencyMet = true;
        if (i.prerequisite != 0)
        {
            // log << logger::DEBUG << "Finding id for prerequisite: " << i.prerequisite;
            auto req = std::find_if(expectations.begin(), expectations.end(),
                [&i](const ExpectationContainer &pre){ return i.prerequisite == pre.id;});

            if (req == expectations.end())
            {
                // log << logger::DEBUG << "Prerequisite id not found...";
                dependencyMet = false;

            }
            if (req != expectations.end() && req->occurence < req->cardinality)
            {
                // log << logger::DEBUG << "prerequisite occurence < cardinality";
                dependencyMet = false;
            }
        }

        if (!dependencyMet)
        {
            index++;
            continue;
        }

        if (i.matcher (buffer, size))
        {
            log << logger::DEBUG << "Matched! id:" << i.id;
            i.occurence++;
            i.action();
            matched = true;
            if (!i.chainable)
            {
                break;
            }
        }
        else
        {
            // log << logger::DEBUG << "Matching failed for index: " << (unsigned int) index;
        }

        index++;
    }

    if (!matched)
    {
        // log << logger::DEBUG << "No match found!";
    }

    sendState = ESendState::WAITING;
    return size;
}

ssize_t EndPointMock::receive(void *buffer, uint32_t size)
{
    // log << logger::DEBUG << "Receive called! Requesting " << size << " byte(s)";
    
    if (toReceive.begin() == toReceive.end())
    {
        // log << logger::DEBUG << "No expectation but message is receive is requested with size " <<
            // size << "!";
        return 0;
    }
    
    auto m = *(toReceive.begin());
    uint32_t rv = 0;

    if (m.size() < size+cursor)
    {
        rv = m.size() - cursor;
        log << logger::DEBUG << "Dispatching request with " << rv << " byte(s):";
        std::memcpy(buffer, m.data() + cursor, rv);
        utils::printRaw(m.data() + cursor, rv);
    }
    else
    {
        log << logger::DEBUG << "Dispatching request with " << size << " byte(s):";
        std::memcpy(buffer, m.data() + cursor, size);
        utils::printRaw(m.data() + cursor, size);
        rv = size;
    }

    cursor += size;

    if (cursor >= m.size())
    {
        toReceive.pop_front();
        cursor = 0;
    }

    return rv;
}


void EndPointMock::queueToReceive(Buffer chunk)
{
    toReceive.push_back(chunk);
}

void EndPointMock::expectSend(uint32_t id, uint32_t prerequisite, bool chainable, uint32_t cardinality, std::function<
    bool(const void *buffer, uint32_t size)> matcher, std::function<void()> action)
{
    ExpectationContainer con;
    con.id = id;
    con.prerequisite = prerequisite;
    con.chainable = chainable;
    con.cardinality = cardinality;
    con.occurence = 0;
    con.matcher = matcher;
    con.action = action;
    expectations.push_back(con);
}

void EndPointMock::waitForAllSending(double milliseconds)
{
    std::chrono::duration<double, std::milli> waitingtime(milliseconds);
    uint32_t index = 0;
    std::ostringstream os;
    bool allMatched = true;

    uint32_t waitct = (uint32_t)(milliseconds/100.0);

    for (uint32_t i = 0; i < waitct; i++)
    {
        bool stop = true;
        for (const auto& ii : expectations)
        {
            if ((ii.cardinality == ii.occurence && ii.cardinality != static_cast<uint32_t>(-1)) ||
                (ii.cardinality == static_cast<uint32_t>(-1) && ii.occurence > 0))
            {
                continue;
            }
            stop = false;
        }
        if (stop)
        {
            break;  
        }
        using namespace std::chrono_literals;   
        std::this_thread::sleep_for(waitingtime/waitct);
    }

    log << logger::WARNING << "checking expectations: ";
    for (const auto& i : expectations)
    {
        log << logger::WARNING << "index:" << (unsigned int)index << " id: " << i.id << ", occurence: " <<
            i.occurence << " cardinality " << i.cardinality;

        if ((i.cardinality == i.occurence && i.cardinality != static_cast<uint32_t>(-1)) ||
            (i.cardinality == static_cast<uint32_t>(-1) && i.occurence > 0))
        {
            index++;
            continue;
        }

        allMatched = false;
        index++;
    }

    if (!allMatched)
    {
        log << logger::ERROR << "Unmatched send expectations above (number of expectations: " << expectations.size() << "):";
        failed("There are unsatified send messages!!");
    }
}

} // namespace server
} // namespace ptree
