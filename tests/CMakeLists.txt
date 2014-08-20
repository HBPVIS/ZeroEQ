# Copyright (c) HBP 2014 Daniel.Nachbaur@epfl.ch
# Change this number when adding tests to force a CMake run: 1

set(TEST_LIBRARIES ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY} zeq)

include(CommonCTest)
