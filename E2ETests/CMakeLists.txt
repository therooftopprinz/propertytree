cmake_minimum_required(VERSION 3.15)
project(propertytree_e2e_test LANGUAGES CXX)

find_package(cum REQUIRED)
find_package(bfc REQUIRED)
find_package(logless REQUIRED)
find_package(propertytree REQUIRED)

# Add your executable
add_executable(propertytree_e2e_test main.cpp)

target_link_libraries(propertytree_e2e_test
    PRIVATE
        bfc::bfc
        cum::cum
        logless::logless
        propertytree::propertytree)