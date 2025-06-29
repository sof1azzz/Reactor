cmake_minimum_required(VERSION 3.16)

project(Reactor)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 添加 include 目录
include_directories(${CMAKE_SOURCE_DIR}/include)

# ================================================================
# 1. 编译静态库 (源文件位于 src/)
# ================================================================
file(GLOB_RECURSE REACTOR_SRC
     ${CMAKE_SOURCE_DIR}/src/*.cpp)

add_library(ReactorLib STATIC ${REACTOR_SRC})

target_link_libraries(ReactorLib pthread)

# ================================================================
# 2. 示例可执行文件 (位于 examples/)
# ================================================================
add_executable(tcpepoll examples/tcpepoll.cpp)
add_executable(client   examples/client.cpp)

target_link_libraries(tcpepoll ReactorLib)
target_link_libraries(client   ReactorLib)

# ================================================================
# 3. Python 测试脚本 (保持不变)
# ================================================================
add_custom_target(tests
  COMMAND ${CMAKE_SOURCE_DIR}/tests/test_client.py
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  COMMENT "Running Python test")