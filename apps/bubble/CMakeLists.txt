# Copyright (c) BBP/EPFL 2011-2014
#                        Stefan.Eilemann@epfl.ch
# All rights reserved. Do not distribute without further notice.

set(BUBBLE_HEADERS)
set(BUBBLE_SOURCES main.cpp)
set(BUBBLE_LINK_LIBRARIES Zerobuf ${Boost_PROGRAM_OPTIONS_LIBRARY}
  ${LUNCHBOX_LIBRARIES})

common_application(bubble)
