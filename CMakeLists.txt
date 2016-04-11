# Copyright (c) HBP 2014-2016 Daniel.Nachbaur@epfl.ch
#                             Stefan.Eilemann@epfl.ch

cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

project(ZeroEQ)
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMake
  ${CMAKE_SOURCE_DIR}/CMake/common)
include(GitExternal)

set(VERSION_MAJOR "0")
set(VERSION_MINOR "6")
set(VERSION_PATCH "0")
set(VERSION_ABI 6)

set(ZEROEQ_DEB_DEPENDS libboost-test-dev libzmq3-dev)

include(Common)

set(COMMON_PROJECT_DOMAIN eu.humanbrainproject)
set(ZEROEQ_DESCRIPTION "ZeroEQ - Zero Event Queue")
set(ZEROEQ_MAINTAINER "Human Brain Project <HBPVis@googlegroups.com>")
set(ZEROEQ_LICENSE LGPL)
set(ZEROEQ_DEPENDENT_LIBRARIES Servus)

common_package(Boost COMPONENTS unit_test_framework)
common_package(FlatBuffers REQUIRED)
common_package(httpxx)
common_package(libzmq REQUIRED)
common_package(Servus REQUIRED)
common_package(Threads REQUIRED)
common_package_post()

add_subdirectory(zeroeq)
add_subdirectory(tests)
add_subdirectory(tools)

set(DOXYGEN_MAINPAGE_MD README.md)
set(DOXYGEN_EXTRA_INPUT ${PROJECT_SOURCE_DIR}/README.md)
include(CPackConfig)
include(DoxygenRule)
