cmake_minimum_required(VERSION 3.15)

project(propertytree_server VERSION 1.2.38 LANGUAGES CXX)
project(propertytree_client VERSION 1.2.38 LANGUAGES CXX)

list(APPEND CMAKE_PREFIX_PATH "install_dependency")

find_package(bfc REQUIRED)
find_package(cum REQUIRED)
find_package(logless REQUIRED)

# server
add_executable(
    propertytree_server
    server/src/main.cpp
    server/src/value_server.cpp)

target_include_directories(
    propertytree_server
    PRIVATE
        server/src
        interface)

target_link_libraries(
    propertytree_server PRIVATE
        bfc::bfc
        cum::cum
        logless::logless)