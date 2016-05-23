# Copyright (c) BBP/EPFL 2015, Stefan.Eilemann@epfl.ch

if(FLATBUFFERS_FOUND)
  set(ZEROEQEVENTGENERATOR_HEADERS)
  set(ZEROEQEVENTGENERATOR_SOURCES eventGenerator.cpp)
  set(ZEROEQEVENTGENERATOR_LINK_LIBRARIES ZeroEQ ZeroEQHBP)
  common_application(zeroeqEventGenerator)
endif()

