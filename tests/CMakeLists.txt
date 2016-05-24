# Copyright (c) HBP 2014-2016 Daniel.Nachbaur@epfl.ch
#                             Stefan.Eilemann@epfl.ch
# Change this number when adding tests to force a CMake run: 5

if(NOT BOOST_FOUND)
  return()
endif()

set(TEST_LIBRARIES ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY} ZeroEQ
  ${CMAKE_THREAD_LIBS_INIT} ${LIBZMQ_LIBRARIES})

if(FLATBUFFERS_FOUND)
  flatbuffers_generate_c_headers(FBS_NEW_EVENT newEvent.fbs)
  flatbuffers_generate_c_headers(FBS_EMPTY_EVENT emptyEvent.fbs)
  flatbuffers_generate_c_headers(FBS_BINARY_EVENT binary.fbs)
  # Order of compilations differs between platforms
  set(EVENT_OUTPUTS ${FBS_NEW_EVENT_OUTPUTS} ${FBS_EMPTY_EVENT_OUTPUTS})
  set(TEST_FILES newEvent.cpp
                 perf.cpp
                 publisher.cpp
                 pubSub.cpp
                 receiver.cpp
                 connection/broker.cpp)
  foreach(EVENT_OUTPUT EVENT_OUTPUTS)
     foreach(TEST_FILE TEST_FILES)
       set_source_files_properties(${TEST_FILE} PROPERTIES OBJECT_DEPENDS
                                   ${EVENT_OUTPUT})
     endforeach(TEST_FILE)
  endforeach(EVENT_OUTPUT)
  list(APPEND TEST_LIBRARIES ZeroEQHBP)
endif()

set(EXCLUDE_FROM_TESTS)
if(NOT HTTPXX_FOUND)
  list(APPEND EXCLUDE_FROM_TESTS http/server.cpp)
endif()

if(NOT FLATBUFFERS_FOUND)
  list(APPEND EXCLUDE_FROM_TESTS hbp/serialization.cpp
                                 newEvent.cpp)
endif()

include(CommonCTest)
