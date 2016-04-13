# Copyright (c) BBP/EPFL 2015, Stefan.Eilemann@epfl.ch

set(ZEROEQCAMERAROTATOR_HEADERS)
set(ZEROEQCAMERAROTATOR_SOURCES cameraRotator.cpp)
set(ZEROEQCAMERAROTATOR_LINK_LIBRARIES ZeroEQ ZeroEQHBP)
common_application(zeroeqCameraRotator)

set(ZEROEQEVENTGENERATOR_HEADERS)
set(ZEROEQEVENTGENERATOR_SOURCES eventGenerator.cpp)
set(ZEROEQEVENTGENERATOR_LINK_LIBRARIES ZeroEQ ZeroEQHBP)
common_application(zeroeqEventGenerator)

common_package(ZeroBuf)
if(TARGET ZeroBufData)
  set(ZEROEQPROGRESSMONITOR_HEADERS)
  set(ZEROEQPROGRESSMONITOR_SOURCES progressMonitor.cpp)
  set(ZEROEQPROGRESSMONITOR_LINK_LIBRARIES ZeroEQ ZeroBuf ZeroBufData)
  common_application(zeroeqProgressMonitor)
endif()
