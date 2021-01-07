cmake_minimum_required(VERSION 3.14)

project(Codenames)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCES 
    src/state.cpp
    src/state_machine.cpp
)

include_directories(include)
find_package(spdlog CONFIG REQUIRED)
find_package(fmt 7 CONFIG REQUIRED)
add_executable(codenames_server src/provider.cpp)
target_sources(codenames_server PUBLIC ${SOURCES})
target_link_libraries(codenames_server codenames_lib fmt::fmt)

find_package(GTest REQUIRED)
add_executable(server_test test/server_test.cpp)
target_sources(server_test PUBLIC ${SOURCES})
target_link_libraries(server_test codenames_server)
gtest_discover_tests(server_test)