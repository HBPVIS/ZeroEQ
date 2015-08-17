# Copyright (c) HBP 2014-2015 Daniel.Nachbaur@epfl.ch
#                             Stefan.Eilemann@epfl.ch
# Change this number when adding tests to force a CMake run: 3

if(NOT BOOST_FOUND)
  return()
endif()

set(TEST_LIBRARIES ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY} zeq zeqHBP)
if(ZEROBUF_FOUND)
  include(zerobufGenerateCxx)
  zerobuf_generate_cxx(TESTEVENT ${CMAKE_CURRENT_BINARY_DIR} ../zeq/detail/echo.fbs)
  include_directories(${CMAKE_CURRENT_BINARY_DIR})

  set(TESTEVENT_LINK_LIBRARIES ZeroBuf)
  set(TESTEVENT_LIBRARY_TYPE STATIC)
  set(TESTEVENT_OMIT_EXPORT ON)
  set(TESTEVENT_INCLUDE_NAME testevent)
  common_library(testevent)

  list(APPEND TEST_LIBRARIES testevent)
else()
  list(APPEND EXCLUDE_FROM_TESTS receiverZeroBuf.cpp)
endif()
flatbuffers_generate_c_headers(FBS_NEW_EVENT newEvent.fbs)
set_source_files_properties(
  newEvent.cpp PROPERTIES OBJECT_DEPENDS ${FBS_NEW_EVENT_OUTPUTS})

flatbuffers_generate_c_headers(FBS_BINARY_EVENT binary.fbs)
set_source_files_properties(
  serialization.cpp PROPERTIES OBJECT_DEPENDS ${FBS_BINARY_EVENT_OUTPUTS})

include(CommonCTest)
