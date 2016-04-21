# Copyright (c) HBP 2014-2015 Daniel.Nachbaur@epfl.ch



list(APPEND CPPCHECK_EXTRA_ARGS
  --suppress=*:${CMAKE_CURRENT_BINARY_DIR}/*_generated.h)

set(ZEROEQ_PUBLIC_HEADERS
  connection/broker.h
  connection/service.h
  log.h
  publisher.h
  receiver.h
  subscriber.h
  types.h
  uri.h)

set(ZEROEQ_HEADERS
  detail/broker.h
  detail/constants.h
  detail/port.h
  detail/sender.h
  detail/socket.h)

set(ZEROEQ_SOURCES
  connection/broker.cpp
  connection/service.cpp
  detail/port.cpp
  detail/sender.cpp
  publisher.cpp
  receiver.cpp
  subscriber.cpp
  uri.cpp)

set(ZEROEQ_LINK_LIBRARIES PUBLIC Servus
                          PRIVATE ${CMAKE_THREAD_LIBS_INIT} ${LIBZMQ_LIBRARIES})

if(FLATBUFFERS_FOUND)
  flatbuffers_generate_c_headers(ZEROEQ_FBS
                                 detail/echo.fbs)
  list(APPEND ZEROEQ_PUBLIC_HEADERS fbevent.h vocabulary.h
                                    ${ZEROEQ_FBS_ZEROEQ_OUTPUTS})
  list(APPEND ZEROEQ_HEADERS detail/fbevent.h detail/vocabulary.h)
  list(APPEND ZEROEQ_SOURCES fbevent.cpp detail/vocabulary.cpp vocabulary.cpp)
  list(APPEND ZEROEQ_LINK_LIBRARIES flatbuffers)
endif()

if(MSVC)
  list(APPEND ZEROEQ_LINK_LIBRARIES Ws2_32)
endif()
if(HTTPXX_FOUND)
  list(APPEND ZEROEQ_PUBLIC_HEADERS http/server.h)
  list(APPEND ZEROEQ_SOURCES http/server.cpp)
  list(APPEND ZEROEQ_LINK_LIBRARIES ${HTTPXX_LIBRARIES})
endif()

common_library(ZeroEQ)

if(FLATBUFFERS_FOUND)
  add_subdirectory(hbp)
endif()
