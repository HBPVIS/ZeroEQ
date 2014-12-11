# Copyright (c) HBP 2014 Juan Hernando <jhernando@fi.upm.es>
# All rights reserved. Do not distribute without further notice.

flatbuffers_generate_c_headers(HBP_FBS
  detail/camera.fbs
  detail/selections.fbs)

list(APPEND ZEQHBP_PUBLIC_HEADERS
  vocabulary.h)

message(${HBP_FBS_OUTPUTS})

set(ZEQHBP_HEADERS
  ${HBP_FBS_OUTPUTS})

set(ZEQHBP_SOURCES
  ${COMMON_SOURCES}
  vocabulary.cpp)

set(ZEQHBP_LINK_LIBRARIES ${LUNCHBOX_LIBRARIES} ${LIBZMQ_LIBRARIES} zeq)
if(MSVC)
  list(APPEND ZEQHBP_LINK_LIBRARIES Ws2_32)
endif()

set(PROJECT_INCLUDE_NAME zeqHBP)
add_definitions(-DZEQ_SHARED)
common_library(zeqHBP)
