#include <functional>
#include <algorithm>
#include <string>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <common/src/TcpEndPoint.hpp>
#include <common/src/Logger.hpp>
#include <common/src/Utils.hpp>
#include <client/src/PTreeClient.hpp>

using namespace ptree;
using Buffer = std::vector<uint8_t>;

class TestRpcForwarder
{
public:
    TestRpcForwarder(unsigned testerIndex, unsigned totalTesters, unsigned rpcIndex,
        std::shared_ptr<client::LocalPTree>& lpt, client::ValueContainerPtr& value):
            mTesterIndex(testerIndex), mTotalTesters(totalTesters), mRpcIndex(rpcIndex), mLpt(lpt), mValue(value),
            log("TestRpcForwarder")
    {}

    Buffer handler(Buffer& bval)
    {
        unsigned targetTester = mTesterIndex+1;
        unsigned targetRpc = mRpcIndex;

        if (targetTester >= mTotalTesters)
        {
            targetTester = 0;
        }
        if (mTesterIndex == 0)
        {
            targetRpc++;
        }

        if (targetRpc < mTotalTesters)
        {
            std::string targetRpcPath = "/tester_" + std::to_string(targetTester)+"/test_rpc_"+
                std::to_string(targetRpc);
            std::string valuePath = "/tester_" + std::to_string(targetTester)+"/test_value_"+
                std::to_string(mRpcIndex);
            auto rpc = mLpt->getRpc(targetRpcPath);
            auto val = mLpt->getValue(valuePath);
            signed &value = val->get<signed>();
            signed &paramValue = *(signed*)(bval.data());
            val->setValue(paramValue);
            log << logger::DEBUG << "TOFILTER: HANDLER OF /tester_"<< mTesterIndex << "/test_rpc_" << mRpcIndex <<
                "SENDING RPC TO " << targetRpcPath << " with param: " << paramValue;
            rpc->call(value);
        }
        else
        {
            std::string valuePath = "/tester_" + std::to_string(targetTester)+"/test_value_"+
                std::to_string(mRpcIndex);
            signed &paramValue = *(signed*)(bval.data());
            auto val = mLpt->getValue(valuePath);
            val->setValue(paramValue);
            log << logger::DEBUG << "TOFILTER: HANDLER OF /tester_"<< mTesterIndex << "/test_rpc_" << mRpcIndex <<
                " FINAL VALUE IS: " << paramValue;
        }

        return Buffer();
    }

    void voidHandler(Buffer&)
    {
        log << logger::ERROR << "TOFILTER: ERROR: VOID HANDLER CALLED!";
    }
private:
    unsigned mTesterIndex;
    unsigned mTotalTesters;
    unsigned mRpcIndex;
    std::shared_ptr<client::LocalPTree>& mLpt;
    client::ValueContainerPtr& mValue;
    logger::Logger log;
};

class RcpHoppingTest
{
public:
    RcpHoppingTest(std::shared_ptr<client::PTreeClient> ptreeClient, unsigned testerIndex, unsigned totalTesters):
        mTesterIndex(testerIndex),
        mTotalTesters(totalTesters),
        mPtreeClient(ptreeClient),
        mLpt(mPtreeClient->getPTree()),
        readyTesters(totalTesters-1),
        log("RcpHoppingTest")
    {
        log << logger::DEBUG << "index: " << mTesterIndex << " in: " << totalTesters;
    }

    void start()
    {
        std::string testerNodePath = "/tester_";
        testerNodePath += std::to_string(mTesterIndex);
        // create tester_T node
        mLpt->createNode(testerNodePath);
        // create test_values and test_rpcs
        using std::placeholders::_1;
        for (unsigned i=0; i<mTotalTesters; i++)
        {
            auto testerValuePath = testerNodePath + "/tester_values_" + std::to_string(i);
            auto testerRpcPath = testerNodePath + "/tester_rpc_" + std::to_string(i);
            auto dval = utils::buildBufferedValue<signed>(-1);
            auto cval = mLpt->createValue(testerValuePath, dval);

            std::shared_ptr<TestRpcForwarder> forwarder = std::make_shared<TestRpcForwarder>
                (mTesterIndex, mTesterIndex, i, mLpt, cval);
            auto fn1 = std::bind(&TestRpcForwarder::handler, *forwarder, _1);
            auto fn2 = std::bind(&TestRpcForwarder::voidHandler, *forwarder, _1);
            mLpt->createRpc(testerRpcPath, fn1, fn2);
        }

        // register ready handlers

        auto testerRpcPath = testerNodePath + "/ready";
        auto fn1 = std::bind(&RcpHoppingTest::readyHandler, this, _1);
        auto fn2 = std::bind(&RcpHoppingTest::readyVoidHandler, this, _1);
        mLpt->createRpc(testerRpcPath, fn1, fn2);

        if (mTesterIndex != 0)
        {
            log << logger::DEBUG << "TOFILTER: READYING /tester_" << mTesterIndex;
            std::string readyPath = "/tester_0/ready";
            auto rpc = mLpt->getRpc(readyPath);
            rpc->call(mTesterIndex);
        }

        for (;;);
    }

    Buffer readyHandler(Buffer& bval)
    {
        if (mTesterIndex == 0)
        {
            unsigned &paramValue = *(unsigned*)(bval.data());
            log << logger::DEBUG << "TOFILTER: /tester_" << paramValue << " is ready!!" ;
            readyTesters[paramValue-1]++;
             bool allOne = std::all_of(readyTesters.begin(), readyTesters.end(), [](const int i){return i==1;});
             if (allOne)
             {
                log << logger::DEBUG << "TOFILTER:  Everyone is ready is ready!!" ;
                for (unsigned i=1; i<mTotalTesters; i++)
                {
                    std::string testerNodePath = "/tester_";
                    testerNodePath += std::to_string(i);
                    std::string readyPath = testerNodePath+"/ready";
                    log << logger::DEBUG << "TOFILTER:  Master to /tester_" << i << "/ready";
                    auto rpc = mLpt->getRpc(readyPath);
                    log << logger::DEBUG << "TOFILTER:  Readying";
                    if (rpc)
                    {
                        rpc->call(0); // TODO: Can't do this one while in handler, receiving is blocked! deadlock
                        log << logger::DEBUG << "TOFILTER: DONE";
                    }
                    else
                    {
                        log << logger::ERROR << "TOFILTER:  RPC NULL";
                    }
                }
             }
        }
        return utils::buildBufferedValue<signed>(0);
    }

    void readyVoidHandler(Buffer&)
    {
        log << logger::ERROR << "TOFILTER: ERROR: VOID HANDLER CALLED!";
    }

private:
    unsigned mTesterIndex;
    unsigned mTotalTesters;
    std::shared_ptr<client::PTreeClient> mPtreeClient;
    std::shared_ptr<client::LocalPTree> mLpt;
    std::vector<int> readyTesters;
    logger::Logger log;
};

int main(int argc, const char* argv[])
{
    logger::Logger log("main");

    if (argc != 3)
    {
        log << logger::ERROR << "Incorrect number of arguments, expected 2, provided " << argc;
        return 1;
    }

    int sockfd;
    struct sockaddr_in server;
     
    //Create socket
    sockfd = socket(AF_INET , SOCK_STREAM , 0);
    if (sockfd == -1)
    {
        log << logger::ERROR << "Could not create socket";
    }

    log << logger::DEBUG << "Socket created!";

    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_family = AF_INET;
    server.sin_port = htons( 6666 );

    //Connect to remote server
    if (connect(sockfd , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        log << logger::DEBUG << "Connect failed. Error";
        return 1;
    }

    log << logger::DEBUG << "Connected!";

    log << logger::DEBUG << "argv[0]: " << argv[0];
    log << logger::DEBUG << "argv[1]: " << argv[1];
    log << logger::DEBUG << "argv[2]: " << argv[2];

    std::shared_ptr<common::TcpEndPoint> sock = std::make_shared<common::TcpEndPoint>(sockfd);
    std::shared_ptr<client::PTreeClient> ptc = std::make_shared<client::PTreeClient>(sock);

    RcpHoppingTest test(ptc, std::stoi(argv[1]), std::stoi(argv[2]));
    test.start();

    return 0;
}