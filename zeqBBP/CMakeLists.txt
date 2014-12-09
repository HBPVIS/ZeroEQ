# Copyright (c) HBP 2014 Juan Hernando <jhernando@fi.upm.es>
# All rights reserved. Do not distribute without further notice.

flatbuffers_generate_c_headers(BBP_FBS
  detail/camera.fbs
  detail/selections.fbs)

list(APPEND ZEQBBP_PUBLIC_HEADERS
  vocabulary.h)

message(${BBP_FBS_OUTPUTS})

set(ZEQBBP_HEADERS
  ${BBP_FBS_OUTPUTS})

set(ZEQBBP_SOURCES
  ${COMMON_SOURCES}
  vocabulary.cpp)

set(ZEQBBP_LINK_LIBRARIES ${LUNCHBOX_LIBRARIES} ${LIBZMQ_LIBRARIES} zeq)
if(MSVC)
  list(APPEND ZEQBBP_LINK_LIBRARIES Ws2_32)
endif()

set(PROJECT_INCLUDE_NAME zeqBBP)
common_library(zeqBBP)
