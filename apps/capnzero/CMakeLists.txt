# Copyright (c) BBP/EPFL 2011-2014
#                        Stefan.Eilemann@epfl.ch
# All rights reserved. Do not distribute without further notice.

set(CAPNZERO_BUBBLE_HEADERS)
set(CAPNZERO_BUBBLE_SOURCES main.cpp)
set(CAPNZERO_BUBBLE_LINK_LIBRARIES Capnzero ${Boost_PROGRAM_OPTIONS_LIBRARY}
  ${LUNCHBOX_LIBRARIES} ${CAPNPROTO_LIBRARIES})

common_application(capnzero_bubble)
