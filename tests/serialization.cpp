
/* Copyright (c) 2014, Human Brain Project
 *                     Juan Hernando <jhernando@fi.upm.es>
 */

#define BOOST_TEST_MODULE zeq_serialization

#include <zeq/zeq.h>

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(test_serialization)
{
    const std::string message("test message");
    const zeq::Event& event = zeq::vocabulary::serializeEcho( message );
    const std::string& deserialized = zeq::vocabulary::deserializeEcho( event );
    BOOST_CHECK_EQUAL( message, deserialized );
}
