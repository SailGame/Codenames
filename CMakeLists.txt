cmake_minimum_required(VERSION 3.14)

project(Codenames)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_BUILD_TYPE Debug)

set(SOURCES 
    src/state.cpp
    src/state_machine.cpp
)

include_directories(include)
find_package(spdlog CONFIG REQUIRED)
find_package(fmt 7 CONFIG REQUIRED)
add_executable(codename_server src/provider.cpp)
target_sources(codename_server PUBLIC ${SOURCES})
target_link_libraries(codename_server codenames_lib fmt::fmt)

# find_package(GTest REQUIRED)
