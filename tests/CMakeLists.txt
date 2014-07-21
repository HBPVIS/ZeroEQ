# Copyright (c) BBP/EPFL 2011-2014, Stefan.Eilemann@epfl.ch
# Change this number when adding tests to force a CMake run: 1

# Includes the install_files function below
include(InstallFiles)

include_directories(${CMAKE_SOURCE_DIR}) # some tests need private headers

# TEST_LIBRARIES variable is used by the CommonCTest.cmake script to link against the given libraries
set(TEST_LIBRARIES ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY} Zerobuf)

# CommonCTest, in the current folder recursively compiles targets for *.cpp files using TEST_LIBRARIES
include(CommonCTest)

# Installs the compiled tests ${TEST_FILES} into the examples directory
#install_files(share/Hello/tests FILES ${TEST_FILES} COMPONENT examples)
