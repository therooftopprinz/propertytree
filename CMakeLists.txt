cmake_minimum_required(VERSION 3.15)

project(propertytree_server VERSION 1.2.38 LANGUAGES CXX)
project(propertytree VERSION 1.2.38 LANGUAGES CXX)

list(APPEND CMAKE_PREFIX_PATH "install")

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
        interface
        server/src)

target_link_libraries(
    propertytree_server PRIVATE
        bfc::bfc
        cum::cum
        logless::logless)

# client
add_library(propertytree STATIC
    client/propertytree/value_client.cpp)

target_include_directories(propertytree PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/client>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/interface>
    $<INSTALL_INTERFACE:include>)

target_link_libraries(
    propertytree PRIVATE
        bfc::bfc
        cum::cum
        logless::logless)

install(
    TARGETS propertytree propertytree_server
    EXPORT propertytreeTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin)

install(
    FILES
        client/propertytree/logger.hpp
        client/propertytree/value_client.hpp
        interface/propertytree/protocol_ng.hpp
    DESTINATION include/propertytree)

install(
    EXPORT propertytreeTargets
    FILE propertytreeTargets.cmake
    NAMESPACE propertytree::
    DESTINATION lib/cmake/propertytree)

include(CMakePackageConfigHelpers)
configure_package_config_file(
    Config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/propertytreeConfig.cmake
    INSTALL_DESTINATION lib/cmake/propertytree)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/propertytreeConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion)

install(
    FILES
    ${CMAKE_CURRENT_BINARY_DIR}/propertytreeConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/propertytreeConfigVersion.cmake
    DESTINATION lib/cmake/propertytree)
    