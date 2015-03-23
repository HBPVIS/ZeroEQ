# Copyright (c) HBP 2014-2015 Daniel.Nachbaur@epfl.ch
# All rights reserved. Do not distribute without further notice.

source_group(\\ FILES CMakeLists.txt)

flatbuffers_generate_c_headers(ZEQ_FBS
  detail/echo.fbs
  detail/exit.fbs
  detail/heartbeat.fbs
  detail/request.fbs
  detail/vocabulary.fbs)

list(APPEND CPPCHECK_EXTRA_ARGS
  --suppress=*:${CMAKE_CURRENT_BINARY_DIR}/*_generated.h)

set(ZEQ_PUBLIC_HEADERS
  ${COMMON_INCLUDES}
  ${ZEQ_FBS_ZEQ_OUTPUTS}
  connection/broker.h
  connection/service.h
  event.h
  eventDescriptor.h
  publisher.h
  receiver.h
  subscriber.h
  types.h
  vocabulary.h)

set(ZEQ_HEADERS
  detail/broker.h
  detail/event.h
  detail/eventDescriptor.h
  detail/socket.h
  detail/vocabulary.h)

set(ZEQ_SOURCES
  ${COMMON_SOURCES}
  connection/broker.cpp
  connection/service.cpp
  detail/vocabulary.cpp
  event.cpp
  eventDescriptor.cpp
  publisher.cpp
  receiver.cpp
  subscriber.cpp
  vocabulary.cpp)

set(ZEQ_LINK_LIBRARIES ${LUNCHBOX_LIBRARIES} ${LIBZMQ_LIBRARIES} ${FLATBUFFERS_LIBRARY})
if(MSVC)
  list(APPEND ZEQ_LINK_LIBRARIES Ws2_32)
endif()

common_library(zeq)
add_subdirectory(hbp)
