

set(HELLO_REPO_URL https://github.com/BlueBrain/Hello.git)
set(HELLO_DEPENDS bluebrain REQUIRED Boost)
set(HELLO_BOOST_COMPONENTS "unit_test_framework program_options")
set(HELLO_DEB_DEPENDS libboost-test-dev libboost-program-options-dev)

if(CI_BUILD_COMMIT)
  set(HELLO_REPO_TAG ${CI_BUILD_COMMIT})
else()
  set(HELLO_REPO_TAG master)
endif()
set(HELLO_FORCE_BUILD ON)
set(HELLO_SOURCE ${CMAKE_SOURCE_DIR})