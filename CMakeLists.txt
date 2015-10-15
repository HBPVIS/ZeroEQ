# Copyright (c) HBP 2014-2015 Daniel.Nachbaur@epfl.ch
#                             Stefan.Eilemann@epfl.ch

cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

project(zeq)
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMake
  ${CMAKE_SOURCE_DIR}/CMake/common)
include(GitExternal)

set(VERSION_MAJOR "0")
set(VERSION_MINOR "4")
set(VERSION_PATCH "0")
set(VERSION_ABI 4)

include(Common)

set(COMMON_PROJECT_DOMAIN eu.humanbrainproject)
set(ZEQ_DESCRIPTION "ZeroEQ - Zero Event Queue")
set(ZEQ_MAINTAINER "Human Brain Project <HBPVis@googlegroups.com>")
set(ZEQ_LICENSE LGPL)
set(ZEQ_DEPENDENT_LIBRARIES Servus)

common_package(Boost COMPONENTS unit_test_framework)
common_package(FlatBuffers REQUIRED)
common_package(libzmq REQUIRED)
common_package(Servus REQUIRED)
common_package(Threads REQUIRED)
common_package(ZeroBuf)
if(ZEROBUF_FOUND)
  list(APPEND ZEQ_DEPENDENT_LIBRARIES ZeroBuf)
endif()
common_package_post()

add_subdirectory(zeq)
add_subdirectory(tests)
add_subdirectory(tools)

set(DOXYGEN_MAINPAGE_MD README.md)
set(DOXYGEN_EXTRA_INPUT ${PROJECT_SOURCE_DIR}/README.md)
include(CPackConfig)
include(DoxygenRule)
