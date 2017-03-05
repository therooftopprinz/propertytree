#include <condition_variable>
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
        unsigned paramValue = *(unsigned*)(bval.data());
        std::thread([this, paramValue](){
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

            if (targetRpc < (mTotalTesters*1))
            {
                std::string targetRpcPath = "/tester_" + std::to_string(targetTester)+"/test_rpc_"+
                    std::to_string(targetRpc);
                std::string valuePath = "/tester_" + std::to_string(mTesterIndex)+"/test_value_"+
                    std::to_string(mRpcIndex);
                auto rpc = mLpt->getRpc(targetRpcPath);

                auto vp = valuePath;
                auto val = mLpt->getValue(vp);
                val->setValue(paramValue);
                unsigned &value = val->get<unsigned>();
                log << logger::DEBUG << "TESTFLOW: HANDLER OF /tester_"<< mTesterIndex << "/test_rpc_" << mRpcIndex <<
                    " SENDING RPC TO " << targetRpcPath << " with param: " << paramValue << ", value is " << value;

                if (rpc)
                {
                    rpc->call(paramValue);
                }
                else
                {
                    log << logger::ERROR << "TESTFLOW:  RPC NULL";
                }
            }
            else
            {
                log << logger::DEBUG << "TESTFLOW: HANDLER OF /tester_"<< mTesterIndex << "/test_rpc_" << mRpcIndex <<
                    " FINAL VALUE IS: " << paramValue;
            }
        }).detach();
        return utils::buildBufferedValue<signed>(0);
    }

    void voidHandler(Buffer&)
    {
        log << logger::ERROR << "TESTFLOW: ERROR: VOID HANDLER CALLED!";
    }
private:
    unsigned mTesterIndex;
    unsigned mTotalTesters;
    unsigned mRpcIndex;
    std::shared_ptr<client::LocalPTree>& mLpt;
    client::ValueContainerPtr& mValue;
    logger::Logger log;
};

template <typename T>
void printContainer(T& t)
{
    logger::Logger log("printContainer");
    std::string content;
    for (auto& i : t)
    {
        content += std::to_string(i) + ", ";
    }
    log << logger::DEBUG << "container: " << content;
}

class RcpHoppingTest
{
public:
    RcpHoppingTest(std::shared_ptr<client::PTreeClient> ptreeClient, unsigned testerIndex, unsigned totalTesters):
        mTesterIndex(testerIndex),
        mTotalTesters(totalTesters),
        mPtreeClient(ptreeClient),
        mLpt(mPtreeClient->getPTree()),
        readyTesters(totalTesters-1),
        allIsReady(false),
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
        log << logger::DEBUG << "TESTFLOW: SETTING UP TESTER " << mTesterIndex;
        for (unsigned i=0; i<(mTotalTesters*1); i++)
        {
            auto testerValuePath = testerNodePath + "/test_value_" + std::to_string(i);
            auto testerRpcPath = testerNodePath + "/test_rpc_" + std::to_string(i);
            auto dval = utils::buildBufferedValue<unsigned>(-1);
            auto cval = mLpt->createValue(testerValuePath, dval);
            log << logger::DEBUG << "TESTFLOW: CREATING: " << testerValuePath;
            log << logger::DEBUG << "TESTFLOW: CREATING: " << testerRpcPath;
            std::shared_ptr<TestRpcForwarder> forwarder = std::make_shared<TestRpcForwarder>
                (mTesterIndex, mTotalTesters, i, mLpt, cval);
            auto fn1 = std::bind(&TestRpcForwarder::handler, *forwarder, _1);
            auto fn2 = std::bind(&TestRpcForwarder::voidHandler, *forwarder, _1);
            mLpt->createRpc(testerRpcPath, fn1, fn2);
        }

        // register ready handlers

        auto testerRpcPath = testerNodePath + "/ready";
        auto fn1 = std::bind(&RcpHoppingTest::readyHandler, this, _1);
        auto fn2 = std::bind(&RcpHoppingTest::readyVoidHandler, this, _1);
        log << logger::DEBUG << "TESTFLOW: CREATING: " << testerRpcPath;
        mLpt->createRpc(testerRpcPath, fn1, fn2);

        if (mTesterIndex != 0)
        {
            log << logger::DEBUG << "TESTFLOW: READYING /tester_" << mTesterIndex;
            std::string readyPath = "/tester_0/ready";
            auto rpc = mLpt->getRpc(readyPath);
            rpc->call(mTesterIndex);
        }

        {
            std::unique_lock<std::mutex> lk(allIsReadyMutex);
            allIsReadyCv.wait(lk, [this]{return this->allIsReady;});
        }
        // TODO: Wait when everyone is ready
        log << logger::DEBUG << "TESTFLOW: EVERONE IS READY FOR RPC TEST!!";

        std::string triggerRpcPath = "/tester_1/test_rpc_0";
        auto rpc = mLpt->getRpc(triggerRpcPath);
        for (signed i=0; i<=10000;i++)
        {
            log << logger::WARNING << "TESTFLOW:  FILL I=" << i;
            if (rpc)
            {
                rpc->call(i);
            }
            else
            {
                log << logger::ERROR << "TESTFLOW:  RPC NULL";
            }

            using namespace std::chrono_literals;
            // std::this_thread::sleep_for(100ms);
        }

        using namespace std::chrono_literals;
        for (;;)std::this_thread::sleep_for(10s);
    }

    Buffer readyHandler(Buffer& bval)
    {
        unsigned &paramValue = *(unsigned*)(bval.data());
        std::thread([this, paramValue](){
            if (mTesterIndex == 0)
            {
                log << logger::DEBUG << "TESTFLOW: /tester_" << paramValue << " is ready!!" ;
                readyTesters[paramValue-1]++;
                 bool allOne = std::all_of(readyTesters.begin(), readyTesters.end(), [](const int i){return i==1;});
                 bool allTwo = std::all_of(readyTesters.begin(), readyTesters.end(), [](const int i){return i==2;});
                 printContainer(readyTesters);
                 if (allOne)
                 {
                    log << logger::DEBUG << "TESTFLOW:  Everyone is ready for subscription!!" ;
                    for (unsigned i=1; i<mTotalTesters; i++)
                    {
                        std::string testerNodePath = "/tester_";
                        testerNodePath += std::to_string(i);
                        std::string readyPath = testerNodePath+"/ready";
                        log << logger::DEBUG << "TESTFLOW:  Master to /tester_" << i << "/ready";

                        std::string rp = readyPath;
                        auto rpc = mLpt->getRpc(rp);
                        if (rpc)
                        {
                            rpc->call(0);
                        }
                        else
                        {
                            log << logger::ERROR << "TESTFLOW:  RPC NULL";
                        }
                    }
                    log << logger::DEBUG << "TESTFLOW: SUBSCRIPTION DONE";
                 }
                 else if (allTwo)
                 {
                    {
                        std::lock_guard<std::mutex> guard(allIsReadyMutex);
                        allIsReady = true;
                    }
                    allIsReadyCv.notify_one();
                 }
            }
            else
            {
                // subscribe all
                log << logger::DEBUG << "TESTFLOW: Subscribing to every test_value." ;
                for (unsigned i=0; i<mTotalTesters; i++)
                {
                    if (i!=mTesterIndex)
                    {
                        std::string testerNodePath = "/tester_";
                        testerNodePath += std::to_string(i);
                        for (unsigned j=0; j<mTotalTesters; j++)
                        {
                            auto valuePath = testerNodePath + "/test_value_" + std::to_string(j);
                            log << logger::DEBUG << "TESTFLOW: Subscribing to " << valuePath;
                            auto val = mLpt->getValue(valuePath);
                            if (val)
                            {
                                val->enableAutoUpdate();// TODO: causes timeout
                            }
                            else 
                            {
                                log << logger::DEBUG << "TESTFLOW: Vaue not found!!";
                            }
                        }
                    }
                }
                log << logger::DEBUG << "TESTFLOW: TESTER HAS SUBSCRIBED INFORMING MASTER.";

                std::string masterReadyPath = "/tester_0/ready";
                auto rpc = mLpt->getRpc(masterReadyPath);
                if (rpc)
                {
                    rpc->call(mTesterIndex);
                }
                else
                {
                    log << logger::ERROR << "TESTFLOW:  RPC NULL";
                }
            }
        }).detach();
        return utils::buildBufferedValue<signed>(0);
    }

    void readyVoidHandler(Buffer&)
    {
        log << logger::ERROR << "TESTFLOW: ERROR: VOID HANDLER CALLED!";
    }

private:
    unsigned mTesterIndex;
    unsigned mTotalTesters;
    std::shared_ptr<client::PTreeClient> mPtreeClient;
    std::shared_ptr<client::LocalPTree> mLpt;
    std::vector<int> readyTesters;
    std::condition_variable allIsReadyCv;
    bool allIsReady;
    std::mutex allIsReadyMutex;
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