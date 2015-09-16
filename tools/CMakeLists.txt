# Copyright (c) BBP/EPFL 2015, Stefan.Eilemann@epfl.ch

set(ZEQCAMERAROTATOR_HEADERS)
set(ZEQCAMERAROTATOR_SOURCES cameraRotator.cpp)
set(ZEQCAMERAROTATOR_LINK_LIBRARIES zeq zeqHBP)
common_application(zeqCameraRotator)

set(ZEQEVENTGENERATOR_HEADERS)
set(ZEQEVENTGENERATOR_SOURCES eventGenerator.cpp)
set(ZEQEVENTGENERATOR_LINK_LIBRARIES zeq zeqHBP)
common_application(zeqEventGenerator)
