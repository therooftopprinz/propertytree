#include <gtest/gtest.h>

#include <propertytree/Client.hpp>
#include <propertytree/Property.hpp>

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    auto rv = RUN_ALL_TESTS();

    // propertytree::ClientConfig config = {"127.0.0.1", 12345};
    // propertytree::Client sut = propertytree::Client(config);

    // sut.root() = 9;
    return rv;
}