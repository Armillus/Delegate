cmake_minimum_required (VERSION 3.8)

project ("Delegate")

find_package(benchmark CONFIG REQUIRED)

include_directories(Delegate PRIVATE include)

add_executable (Delegate "main.cpp" "include/Delegate/Delegate.hpp")

target_link_libraries(Delegate PRIVATE benchmark::benchmark benchmark::benchmark_main)
target_compile_features(Delegate PRIVATE cxx_std_17)

# TODO: Add tests and install targets if needed.