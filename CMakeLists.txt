cmake_minimum_required (VERSION 3.8)

set(APP Delegate)

project(${APP} VERSION "1.0.0")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(CMAKE_PROJECT_NAME STREQUAL ${APP})
	set(BUILD_SUBDIRECTORIES TRUE)
else()
	set(BUILD_SUBDIRECTORIES FALSE)
endif()

if(${BUILD_SUBDIRECTORIES})
	add_subdirectory(benchmarks)
endif()

add_library(${APP} INTERFACE ${PROJECT_SOURCE_DIR}/include/Delegate/Delegate.hpp)

target_compile_features(${APP} INTERFACE cxx_std_17)
target_include_directories(${APP} INTERFACE include)