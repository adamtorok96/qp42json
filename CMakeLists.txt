cmake_minimum_required(VERSION 3.8)
project(qp42json)

set(CMAKE_CXX_STANDARD 11)

include(FindLua REQUIRED)

include_directories(${LUA_INCLUDE_DIR})

set(SOURCE_FILES main.cpp)
add_executable(qp42json ${SOURCE_FILES})
target_link_libraries(qp42json ${LUA_LIBRARIES})