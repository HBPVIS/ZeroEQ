
/* Copyright (c) 2014, Human Brain Project
 *                     Daniel Nachbaur <daniel.nachbaur@epfl.ch>
 */

#include <zeq/zeq.h>
#include <lunchbox/uri.h>

#define BOOST_TEST_MODULE serialization
#include <boost/test/unit_test.hpp>
#include <boost/bind.hpp>

BOOST_AUTO_TEST_CASE(test_serialization)
{
    const std::vector< float > camera( 16, 42 );
    const zeq::Event& event = zeq::vocabulary::serializeCamera( camera );
    const std::vector< float >& deserialized =
            zeq::vocabulary::deserializeCamera( event );
    BOOST_CHECK_EQUAL_COLLECTIONS( camera.begin(), camera.end(),
                                   deserialized.begin(), deserialized.end( ));
}

BOOST_AUTO_TEST_CASE(test_invalid_serialization)
{
    const std::vector< float > invalidCamera( 42 );
    const zeq::Event& event =
            zeq::vocabulary::serializeCamera( invalidCamera);
    BOOST_CHECK_EQUAL( event.getType(), zeq::vocabulary::EVENT_INVALID );
    BOOST_CHECK_EQUAL( event.getSize(), 0 );
}
