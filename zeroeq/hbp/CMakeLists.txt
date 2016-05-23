# Copyright (c) HBP 2014-2015 Juan Hernando <jhernando@fi.upm.es>
#                             Daniel Nachbaur <daniel.nachbaur@epfl.ch>
# All rights reserved. Do not distribute without further notice.

flatbuffers_generate_c_headers(HBP_FBS
  detail/camera.fbs
  detail/cellSetBinaryOp.fbs
  detail/selections.fbs
)

set(ZEROEQHBP_PUBLIC_HEADERS enums.h vocabulary.h ${HBP_FBS_ZEROEQ_OUTPUTS})
set(ZEROEQHBP_SOURCES vocabulary.cpp)
set(ZEROEQHBP_LINK_LIBRARIES PUBLIC ZeroEQ)
if(MSVC)
  list(APPEND ZEROEQHBP_LINK_LIBRARIES PRIVATE Ws2_32)
endif()

list(APPEND CPPCHECK_EXTRA_ARGS
  --suppress=*:${CMAKE_CURRENT_BINARY_DIR}/*_generated.h)

set(ZEROEQHBP_INCLUDE_NAME zeroeq/hbp)
set(ZEROEQHBP_NAMESPACE zeroeqhbp)
add_definitions(-DZEROEQ_SHARED)
common_library(ZeroEQHBP)
