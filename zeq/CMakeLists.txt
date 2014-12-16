# Copyright (c) HBP 2014 Daniel.Nachbaur@epfl.ch
# All rights reserved. Do not distribute without further notice.

source_group(\\ FILES CMakeLists.txt)

flatbuffers_generate_c_headers(ZEQ_FBS detail/echo.fbs)

set(ZEQ_PUBLIC_HEADERS
  ${COMMON_INCLUDES}
  event.h
  publisher.h
  receiver.h
  subscriber.h
  types.h
  vocabulary.h)

set(ZEQ_HEADERS
  ${ZEQ_FBS_OUTPUTS}
  detail/broker.h
  detail/event.h
  detail/serialization.h
  detail/socket.h)

set(ZEQ_SOURCES
  ${COMMON_SOURCES}
  event.cpp
  publisher.cpp
  receiver.cpp
  subscriber.cpp
  vocabulary.cpp)

set(ZEQ_LINK_LIBRARIES ${LUNCHBOX_LIBRARIES} ${LIBZMQ_LIBRARIES})
if(MSVC)
  list(APPEND ZEQ_LINK_LIBRARIES Ws2_32)
endif()

common_library(zeq)
add_subdirectory(hbp)