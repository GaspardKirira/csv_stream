cmake_minimum_required(VERSION 3.20)
project(csv_stream LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(csv_stream INTERFACE)
add_library(csv_stream::csv_stream ALIAS csv_stream)

target_include_directories(csv_stream INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

if (MSVC)
  target_compile_options(csv_stream INTERFACE /W4 /permissive-)
else()
  target_compile_options(csv_stream INTERFACE -Wall -Wextra -Wpedantic)
endif()

include(CTest)
enable_testing()

add_executable(csv_stream_basic_test tests/test_basic.cpp)
target_link_libraries(csv_stream_basic_test PRIVATE csv_stream::csv_stream)
add_test(NAME csv_stream.basic COMMAND csv_stream_basic_test)
