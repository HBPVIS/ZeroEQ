# Copyright (c) BBP/EPFL 2011-2014
#                        Stefan.Eilemann@epfl.ch
# All rights reserved. Do not distribute without further notice.

source_group(\\ FILES CMakeLists.txt)

CAPNP_TARGET(SCHEMER data.capnp expand.capnp exit.capnp wildcard.capnp)

set(CAPNZERO_PUBLIC_HEADERS ${COMMON_INCLUDES} world.h)
set(CAPNZERO_HEADERS ${SCHEMER_OUTPUTS})
set(CAPNZERO_SOURCES ${COMMON_SOURCES} world.cpp)
set(CAPNZERO_LINK_LIBRARIES ${LIBZMQ_LIBRARIES} ${Boost_PROGRAM_OPTIONS_LIBRARY})

common_library(Capnzero)
