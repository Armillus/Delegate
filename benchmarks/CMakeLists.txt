cmake_minimum_required (VERSION 3.8)

set(APP DelegateBenchmarks)

project(${APP} VERSION "1.0.0" LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Install Google Benchmark with vcpkg or conan
find_package(benchmark CONFIG REQUIRED)

add_executable(${APP} "benchmarks.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/../include/Delegate/Delegate.hpp")

target_include_directories(${APP} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include)
target_link_libraries(${APP} PRIVATE benchmark::benchmark benchmark::benchmark_main)