cmake_minimum_required(VERSION 3.2)

project(dlog)

find_package(Threads REQUIRED)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
set(bin_name "dlog")

set(
	CMAKE_RUNTIME_OUTPUT_DIRECTORY
	"${CMAKE_CURRENT_SOURCE_DIR}/bin"
	CACHE
	PATH
	"Executable output directory"
)

set(
	CMAKE_CXX_COMPILER
	"/usr/bin/clang++"
	CACHE
	STRING
	"C++ Compiler"
	FORCE
	)

include_directories(
	include
	)

set(
	src_list
	include/dlog.hpp
	src/example.cpp
	)

add_executable(${bin_name} ${src_list})
target_link_libraries(${bin_name} ${CMAKE_THREAD_LIBS_INIT})
