#include <cstring>
#include <cstdlib>
#include <ctime>

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <server/src/PTree.hpp>
#include <server/src/PTreeTcpServer.hpp>
#include <common/src/Logger.hpp>

using namespace testing;

namespace ptree
{
namespace core
{

struct PTreeTests : public ::testing::Test
{
    PTreeTests():
        log("TEST")
    {}

    void TearDown()
    {
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(1ms);
    }

    ValueContainer makeValue(void* value, size_t size)
    {
        ValueContainer container;
        container.resize(size);
        std::memcpy(container.data(), value, size);
        return container;
    }

    template<class T>
    T* reconstructValue(void* value)
    {
        return reinterpret_cast<T*>(value);
    }

    const protocol::Uuid uuid = 100; 
    logger::Logger log;

};

template <typename T>
class ValueSetGetTest : public ::testing::Test {
  
};

TYPED_TEST_CASE_P(ValueSetGetTest);

TYPED_TEST_P(ValueSetGetTest, shouldSetGetNative) {
    auto timeValue = time(NULL);

    // std::cout << "time:      " << timeValue << std::endl;
    srand(timeValue);
    double randomVal = (rand()/1024.1)*(timeValue%2?-1:1);

    // std::cout << "random:    " << randomVal << std::endl;
    TypeParam tval = static_cast<TypeParam>(randomVal);
    // std::cout << "converted: " << tval << std::endl;

    const protocol::Uuid uuid = 100;
    Value value(uuid, NodePtr());

    value.setValue(tval);
    EXPECT_EQ(value.getValue<TypeParam>(), tval);

}

REGISTER_TYPED_TEST_CASE_P(ValueSetGetTest, shouldSetGetNative);

typedef ::testing::Types<uint32_t, uint64_t, int32_t, int64_t, double, float> MyTypes;
INSTANTIATE_TYPED_TEST_CASE_P(ValueSetGetTest, ValueSetGetTest, MyTypes);


TEST_F(PTreeTests, shouldSetGetBuffered)
{
    struct TestStruct
    {
        int a;
        double b;
        char c[4];
    };

    TestStruct testvalue;
    testvalue.a = 42;
    testvalue.b = 42.0;
    testvalue.c[0] = 'a';
    testvalue.c[1] = 'b';
    testvalue.c[2] = 'c';
    testvalue.c[3] = 0;

    ValueContainer data = makeValue(&testvalue, sizeof(testvalue));

    Value value(uuid, NodePtr());

    value.setValue(testvalue);

    const ValueContainer& testget = value.getValue();

    EXPECT_EQ(testvalue.a, reinterpret_cast<const TestStruct*>(testget.data())->a);
    EXPECT_EQ(testvalue.b, reinterpret_cast<const TestStruct*>(testget.data())->b);
}

struct WatcherMock
{
    MOCK_METHOD1(watch, bool(ValueWkPtr));
};

TEST_F(PTreeTests, shouldCallWatcherWhenModified)
{
    using std::placeholders::_1;
    using ::testing::DoAll;

    WatcherMock watcher;
    ValueWatcher watcherfn = std::bind(&WatcherMock::watch, &watcher, _1);

    ValuePtr value = std::make_shared<Value>(42, NodePtr());
    ValueWkPtr container;

    EXPECT_CALL(watcher, watch(_))
        .WillOnce(DoAll(SaveArg<0>(&container), Return(true)));

    value->addWatcher(1, watcherfn);
    value->setValue<uint32_t>(420u);

    EXPECT_EQ(*reconstructValue<uint32_t>(container.lock()->getValue().data()), 420u);
}

TEST_F(PTreeTests, shouldNotCallWatcherWhenRemoved)
{
    using std::placeholders::_1;
    WatcherMock watcher1;
    WatcherMock watcher2;
    ValueWatcher watcher1fn = std::bind(&WatcherMock::watch, &watcher1, _1); 
    ValueWatcher watcher2fn = std::bind(&WatcherMock::watch, &watcher2, _1); 

    ValuePtr value = std::make_shared<Value>(42, NodePtr());

    EXPECT_CALL(watcher1, watch(_))
        .Times(2)
        .WillRepeatedly(Return(true));
    EXPECT_CALL(watcher2, watch(_))
        .Times(1)
        .WillRepeatedly(Return(true));

    value->addWatcher(1, watcher1fn);
    value->addWatcher(2, watcher2fn);
    value->setValue<uint32_t>(420u);
    value->removeWatcher(2);
    value->setValue<uint32_t>(200u);

}

TEST_F(PTreeTests, shouldNotAddSameId)
{
    using std::placeholders::_1;
    WatcherMock watcher1;
    ValueWatcher watcher1fn = std::bind(&WatcherMock::watch, &watcher1, _1); 
    Value value(42, NodePtr());

    EXPECT_TRUE(value.addWatcher(1, watcher1fn));
    EXPECT_FALSE(value.addWatcher(1, watcher1fn));
}

TEST_F(PTreeTests, shouldNotRemoveIfNone)
{
    using std::placeholders::_1;
    WatcherMock watcher1;
    ValueWatcher watcher1fn = std::bind(&WatcherMock::watch, &watcher1, _1); 
    ValuePtr value = std::make_shared<Value>(42, NodePtr());

    EXPECT_TRUE(value->addWatcher(1, watcher1fn));
    EXPECT_TRUE(value->addWatcher(2, watcher1fn));
    EXPECT_TRUE(value->removeWatcher(2));
    EXPECT_FALSE(value->removeWatcher(2));
}

TEST_F(PTreeTests, shouldCreateAndGetNode)
{
    using std::placeholders::_1;
    NodePtr root = std::make_shared<Node>(uuid, NodePtr());

    const std::string parentPath = "parent";
    const std::string childrenPath = "children";
    const std::string value1Path = "value1";
    const std::string value2Path = "value2";

    auto node1 = root->createProperty<Node>(parentPath, 100);
    auto node2 = node1->createProperty<Node>(childrenPath, 101);
    auto valu1 = node1->createProperty<Value>(value1Path, 102);
    auto valu2 = node2->createProperty<Value>(value2Path, 103);

    auto gNode1 = root->getProperty<Node>(parentPath);
    auto gNode2 = node1->getProperty<Node>(childrenPath);
    auto gValu1 = node1->getProperty<Value>(value1Path);
    auto gValu2 = node2->getProperty<Value>(value2Path);

    auto test3 = root->getProperty<Value>(parentPath);
    auto test4 = root->getProperty<Node>(parentPath);

    EXPECT_EQ(node1, gNode1);
    EXPECT_EQ(node2, gNode2);
    EXPECT_EQ(valu1, gValu1);
    EXPECT_EQ(valu2, gValu2);
}


TEST_F(PTreeTests, getNodeByPath)
{
    using std::placeholders::_1;
    IIdGeneratorPtr idgen = std::make_shared<IdGenerator>();
    PTree ptree(idgen);

    auto root = ptree.getNodeByPath("/");

    const std::string fcsName = "FCS";
    const std::string aileronName = "AILERON";
    const std::string valueName = "VALUE";

    auto fGen = root->createProperty<Node>(fcsName, 100);
    auto sGen = fGen->createProperty<Node>(aileronName, 101);
    auto val = sGen->createProperty<Value>(valueName, 102);
    val->setValue<uint32_t>(420);

    auto gnode = ptree.getNodeByPath("/FCS/AILERON");
    auto val2 = gnode->getProperty<Value>("VALUE");

    ptree.getPTreeInfo();
    EXPECT_EQ(val, val2);
}

TEST_F(PTreeTests, getPTreeInfo)
{
    using std::placeholders::_1;
    IIdGeneratorPtr idgen = std::make_shared<IdGenerator>();
    PTree ptree(idgen);

    auto fcs =  ptree.createProperty<Node>("/FCS");
    auto sens = ptree.createProperty<Node>("/SENSOR");
    auto aile = ptree.createProperty<Node>("/FCS/AILERON");
    auto acel = ptree.createProperty<Node>("/SENSOR/ACCELEROMETER");
    auto ther = ptree.createProperty<Node>("/SENSOR/THERMOMETER");
    auto val1 = ptree.createProperty<Value>("/SENSOR/THERMOMETER/VALUE");
    auto val2 = ptree.createProperty<Value>("/SENSOR/ACCELEROMETER/VALUE");
    auto val3 = ptree.createProperty<Value>("/FCS/AILERON/CURRENT_DEFLECTION");
    auto val4 = ptree.createProperty<Value>("/FCS/AILERON/TRIM");

    struct IsInTheInfoList
    {
        IsInTheInfoList(std::string path, uint32_t uuid, uint8_t type):
            path(path), uuid((protocol::Uuid)uuid), type((protocol::PropertyType)type)
        {}
        bool operator()(const std::tuple<std::string, protocol::Uuid, protocol::PropertyType>& in)
        {
            return std::get<0>(in)==path && std::get<1>(in)==uuid  && std::get<2>(in)==type;
        }
        std::string path;
        protocol::Uuid uuid;
        protocol::PropertyType type;
    };

    auto infolist = ptree.getPTreeInfo();
    EXPECT_NE(std::find_if(infolist.begin(), infolist.end(),IsInTheInfoList("/FCS",100,1)), infolist.end());
    EXPECT_NE(std::find_if(infolist.begin(), infolist.end(),IsInTheInfoList("/FCS/AILERON",102,1)), infolist.end());
    EXPECT_NE(std::find_if(infolist.begin(), infolist.end(),IsInTheInfoList("/FCS/AILERON/CURRENT_DEFLECTION",107,0)), infolist.end());
    EXPECT_NE(std::find_if(infolist.begin(), infolist.end(),IsInTheInfoList("/FCS/AILERON/TRIM",108,0)), infolist.end());
    EXPECT_NE(std::find_if(infolist.begin(), infolist.end(),IsInTheInfoList("/SENSOR",101,1)), infolist.end());
    EXPECT_NE(std::find_if(infolist.begin(), infolist.end(),IsInTheInfoList("/SENSOR/ACCELEROMETER",103,1)), infolist.end());
    EXPECT_NE(std::find_if(infolist.begin(), infolist.end(),IsInTheInfoList("/SENSOR/ACCELEROMETER/VALUE",106,0)), infolist.end());
    EXPECT_NE(std::find_if(infolist.begin(), infolist.end(),IsInTheInfoList("/SENSOR/THERMOMETER",104,1)), infolist.end());
    EXPECT_NE(std::find_if(infolist.begin(), infolist.end(),IsInTheInfoList("/SENSOR/THERMOMETER/VALUE",105,0)), infolist.end());
    for (const auto& i : infolist)
    {
        log << logger::DEBUG << "ENTRY path " << std::get<0>(i) << " uuid " <<  (uint32_t)std::get<1>(i) << " type " <<  (uint32_t)std::get<2>(i);
    }
}

TEST_F(PTreeTests, getPropertyByPath)
{
    using std::placeholders::_1;    
    IIdGeneratorPtr idgen = std::make_shared<IdGenerator>();
    PTree ptree(idgen);

    const std::string fcsName = "FCS";
    const std::string aileronName = "AILERON";
    const std::string valueName = "VALUE";
    auto root = ptree.getNodeByPath("/");
    auto fGen = root->createProperty<Node>(fcsName, 100);
    auto sGen = fGen->createProperty<Node>(aileronName, 101);
    auto val = sGen->createProperty<Value>(valueName, 102);
    val->setValue<uint32_t>(420);

    auto val2 = ptree.getPropertyByPath<Value>("/FCS/AILERON/VALUE");
    EXPECT_EQ(val, val2);
}

TEST_F(PTreeTests, shouldDeleteOnPTree)
{
    using std::placeholders::_1;
    IIdGeneratorPtr idgen = std::make_shared<IdGenerator>();
    PTree ptree(idgen);

    const std::string valuePath = "/Value";
    auto fcs =  ptree.createProperty<Node>(valuePath);

    ptree.deleteProperty(valuePath);

    EXPECT_THROW(ptree.getPropertyByPath<Value>(valuePath), ObjectNotFound);
}

TEST_F(PTreeTests, shouldDelete2OnPTree)
{
    using std::placeholders::_1;
    IIdGeneratorPtr idgen = std::make_shared<IdGenerator>();
    PTree ptree(idgen);

    const std::string valuePath = "/Value";
    auto fcs =  ptree.createProperty<Node>(valuePath);

    ptree.deleteProperty(fcs.first);

    EXPECT_THROW(ptree.getPropertyByPath<Value>(valuePath), ObjectNotFound);
}

} // namespace core
} // namespace ptree