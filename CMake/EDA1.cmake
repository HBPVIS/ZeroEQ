

set(EDA1_REPO_URL https://github.com/HBPVIS/EDA1.git)
set(EDA1_DEPENDS REQUIRED libzmq flatbuffers Boost Lunchbox)
set(EDA1_OPTIONAL ON)
set(EDA1_BOOST_COMPONENTS "unit_test_framework program_options")
set(EDA1_DEB_DEPENDS libboost-test-dev libboost-program-options-dev)

if(CI_BUILD_COMMIT)
  set(EDA1_REPO_TAG ${CI_BUILD_COMMIT})
else()
  set(EDA1_REPO_TAG master)
endif()
set(EDA1_FORCE_BUILD ON)
set(EDA1_SOURCE ${CMAKE_SOURCE_DIR})