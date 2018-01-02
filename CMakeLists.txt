cmake_minimum_required(VERSION 3.2)

project(dlog)

include(ExternalProject)

find_package(Threads REQUIRED)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
set(bin_name "dlog")
set(tp_debug ON CACHE BOOL "Threadpool debug output" FORCE)

if (NOT DEFINED download_single_header_libs)
	message(STATUS "*** Downloading single-header libraries...")

	file(DOWNLOAD
		https://raw.githubusercontent.com/cantordust/threadpool/master/include/threadpool.hpp
		${CMAKE_CURRENT_SOURCE_DIR}/include/threadpool.hpp
	)

    set(download_single_header_libs
		FALSE
		CACHE
		BOOL
		"Download single-header libraries"
		)
endif()

set(
	CMAKE_RUNTIME_OUTPUT_DIRECTORY
	${CMAKE_CURRENT_SOURCE_DIR}/bin
	CACHE
	PATH
	"Executable output directory"
)

if(${tp_debug})
	add_definitions(-DTP_DEBUG)
endif()

set(
	CMAKE_CXX_COMPILER
	"clang++"
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
	include/threadpool.hpp
	include/dlog.hpp
	src/example.cpp
	)

add_executable(${bin_name} ${src_list})
target_link_libraries(${bin_name} ${CMAKE_THREAD_LIBS_INIT})
