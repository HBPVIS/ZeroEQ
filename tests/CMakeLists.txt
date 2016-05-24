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
  set_source_files_properties(newEvent.cpp
      PROPERTIES OBJECT_DEPENDS ${FBS_NEW_EVENT_OUTPUTS})

  flatbuffers_generate_c_headers(FBS_EMPTY_EVENT emptyEvent.fbs)
  set_source_files_properties(pubSub.cpp subscriber.cpp
      PROPERTIES OBJECT_DEPENDS ${FBS_EMPTY_EVENT_OUTPUTS})
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
