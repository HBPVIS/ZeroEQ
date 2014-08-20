# Copyright (c) HBP 2014 Daniel.Nachbaur@epfl.ch
# All rights reserved. Do not distribute without further notice.

source_group(\\ FILES CMakeLists.txt)

FLATC_TARGET(SCHEMER detail/camera.fbs)

set(ZEQ_PUBLIC_HEADERS ${COMMON_INCLUDES} broker.h event.h types.h vocabulary.h)
set(ZEQ_HEADERS ${SCHEMER_OUTPUTS} detail/event.h detail/serialization.h)
set(ZEQ_SOURCES ${COMMON_SOURCES} broker.cpp event.cpp vocabulary.cpp)
set(ZEQ_LINK_LIBRARIES ${LUNCHBOX_LIBRARIES} ${LIBZMQ_LIBRARIES})

common_library(zeq)
