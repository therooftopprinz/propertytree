#ifndef SERVER_UT_ENDPOINTMOCK_HPP_
#define SERVER_UT_ENDPOINTMOCK_HPP_

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <list>
#include <vector>
#include <cstring>
#include <functional>
#include <server/src/PTreeTcpServer.hpp>
#include <server/src/Logger.hpp>
#include <server/src/Types.hpp>
#include <interface/protocol.hpp>

namespace ptree
{
namespace server
{

typedef std::function<bool(const void*,uint32_t)> MatcherFunctor;
typedef std::function<void()> ActionFunctor;


class DefaultAction
{
public:
    inline static ActionFunctor get()
    {
        return std::bind(&DefaultAction::action);
    }

private:
    inline static void action()
    {

    }
};
class EndPointMock : public IEndPoint
{
public:
    EndPointMock():
        IEndPoint(),
        cursor(0),
        log(logger::Logger("EndPointMock"))
    {
    }
    ssize_t receive(void *buffer, uint32_t size);
    ssize_t send(const void *buffer, uint32_t size);
    void expectSend(uint32_t id, uint32_t prerequisite, bool chainable, uint32_t cardinality, std::function<
    bool(const void *buffer, uint32_t size)> matcher, std::function<void()> action);
    void queueToReceive(std::vector<uint8_t> chunk);

    void waitForAllSending(double milliseconds);

    // MOCK_METHOD1(setReceiveTimeout, void(uint32_t));

private:
    void failed(std::string msg);

    struct ExpectationContainer
    {
        uint32_t cardinality = 0;
        uint32_t occurence = 0;
        MatcherFunctor matcher;
        ActionFunctor action;
        uint32_t id = 0;
        uint32_t prerequisite = 0;
        bool chainable = false;
    };

    enum class ESendState {WAITING, INCOMPLETE};
    ESendState sendState = ESendState::WAITING;
    uint32_t sendSize = 0;
    uint32_t expectedSendSize = 0;
    uint32_t cursor;
    std::list<Buffer> toReceive;
    std::vector<uint8_t> sendBuffer;
    uint8_t* sendBufferCursor = nullptr;
    std::list<ExpectationContainer> expectations;
    logger::Logger log;
};

} // namespace server
} // namespace ptree
#endif // SERVER_UT_ENDPOINTMOCK_HPP_