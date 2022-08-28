cmake_minimum_required (VERSION 3.21)

set(APP Delegate)

project(${APP} VERSION "1.0.1")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(CMAKE_PROJECT_NAME STREQUAL ${APP})
	set(BUILD_SUBDIRECTORIES TRUE)
else()
	set(BUILD_SUBDIRECTORIES FALSE)
endif()

if(${BUILD_SUBDIRECTORIES})
	# TODO: Add tests and remove benchmarks as they can't be compiled with github actions
	add_subdirectory(benchmarks) 
endif()

add_library(${APP} INTERFACE ${PROJECT_SOURCE_DIR}/include/Delegate/Delegate.hpp)

target_compile_features(${APP} INTERFACE cxx_std_20)
target_include_directories(${APP} INTERFACE include)

add_executable(test WIN32 
	${PROJECT_SOURCE_DIR}/main.cpp
	${PROJECT_SOURCE_DIR}/include/Delegate/Delegate.hpp
)

# target_link_libraries(test "Delegate")

target_compile_features(test PRIVATE cxx_std_20)
target_compile_options(test PRIVATE /Wall /d2cgsummary /Bt /sdl)
target_include_directories(test PRIVATE include)
set_target_properties(test PROPERTIES LINK_FLAGS /SUBSYSTEM:CONSOLE)