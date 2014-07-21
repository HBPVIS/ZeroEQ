# Copyright (c) BBP/EPFL 2011-2014
#                        Stefan.Eilemann@epfl.ch
# All rights reserved. Do not distribute without further notice.

source_group(\\ FILES CMakeLists.txt)

FLATC_TARGET(SCHEMER data.fbs exit.fbs expand.fbs wildcard.fbs)

set(ZEROBUF_PUBLIC_HEADERS ${COMMON_INCLUDES} world.h)
set(ZEROBUF_HEADERS ${SCHEMER_OUTPUTS})
set(ZEROBUF_SOURCES ${COMMON_SOURCES} world.cpp)
set(ZEROBUF_LINK_LIBRARIES ${LIBZMQ_LIBRARIES} ${Boost_PROGRAM_OPTIONS_LIBRARY})

if(WIN32)
  list(APPEND ZEROBUF_LINK_LIBRARIES ws2_32 mswsock)
endif(WIN32)
if(LINUX)
  list(APPEND ZEROBUF_LINK_LIBRARIES dl rt)
endif()

common_library(Zerobuf)
