
/* Copyright (c) 2014, Human Brain Project
 *                     Daniel Nachbaur <daniel.nachbaur@epfl.ch>
 *                     Stefan.Eilemann@epfl.ch
 */

#include "broker.h"

#include <boost/bind.hpp>

BOOST_AUTO_TEST_CASE(test_create_uri_publisher)
{
    zeq::Publisher publisher( lunchbox::URI( "foo://" ));
}

BOOST_AUTO_TEST_CASE(test_create_invalid_uri_publisher)
{
    // invalid URI, no hostname allowed
    BOOST_CHECK_THROW(
        zeq::Publisher publisher( lunchbox::URI( "foo://localhost" )),
        std::runtime_error );
}

BOOST_AUTO_TEST_CASE(test_publish)
{
    zeq::Publisher publisher( lunchbox::URI( buildURI( "*" )));
    BOOST_CHECK( publisher.publish( zeq::vocabulary::serializeCamera( camera)));
}

BOOST_AUTO_TEST_CASE(test_publish_receive)
{
    lunchbox::RNG rng;
    const unsigned short port = (rng.get<uint16_t>() % 60000) + 1024;
    const std::string& portStr = boost::lexical_cast< std::string >( port );
    zeq::Subscriber subscriber( lunchbox::URI( "foo://localhost:" + portStr ));
    BOOST_CHECK( subscriber.registerHandler( zeq::vocabulary::EVENT_CAMERA,
                                             boost::bind( &onEvent, _1 )));

    zeq::Publisher publisher( lunchbox::URI( "foo://*:" + portStr ));

    bool received = false;
    for( size_t i = 0; i < 10; ++i )
    {
        BOOST_CHECK( publisher.publish(
                         zeq::vocabulary::serializeCamera( camera )));

        if( subscriber.receive( 100 ))
        {
            received = true;
            break;
        }
    }
    BOOST_CHECK( received );
}

BOOST_AUTO_TEST_CASE(test_publish_receive_zeroconf)
{
    zeq::Publisher publisher( lunchbox::URI( "foo://" ));
    zeq::Subscriber subscriber( lunchbox::URI( "foo://" ));

    BOOST_CHECK( subscriber.registerHandler( zeq::vocabulary::EVENT_CAMERA,
                                             boost::bind( &onEvent, _1 )));
    bool received = false;
    for( size_t i = 0; i < 10; ++i )
    {
        BOOST_CHECK( publisher.publish(
                         zeq::vocabulary::serializeCamera( camera )));

        if( subscriber.receive( 100 ))
        {
            received = true;
            break;
        }
    }
    BOOST_CHECK( received );
}
