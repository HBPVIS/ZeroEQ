
/* Copyright (c) 2014, Human Brain Project
 *                     Stefan.Eilemann@epfl.ch
 */

#include "broker.h"

#include <lunchbox/sleep.h>
#include <lunchbox/thread.h>
#include <lunchbox/servus.h>
#include <boost/bind.hpp>

bool gotOne = false;
bool gotTwo = false;

void onEvent1( const zeq::Event& )
{
    gotOne = true;
}
void onEvent2( const zeq::Event& )
{
    gotTwo = true;
}

BOOST_AUTO_TEST_CASE(test_two_subscribers)
{
    lunchbox::RNG rng;
    const unsigned short port = (rng.get<uint16_t>() % 60000) + 1024;
    const std::string& portStr = boost::lexical_cast< std::string >( port );
    zeq::Subscriber subscriber1( lunchbox::URI( "foo://localhost:" + portStr ));
    zeq::Subscriber subscriber2( lunchbox::URI( "foo://localhost:" + portStr ),
                                 subscriber1 );

    BOOST_CHECK( subscriber1.registerHandler( zeq::vocabulary::EVENT_CAMERA,
                                              boost::bind( &onEvent1, _1 )));
    BOOST_CHECK( subscriber2.registerHandler( zeq::vocabulary::EVENT_CAMERA,
                                              boost::bind( &onEvent2, _1 )));

    zeq::Publisher publisher( lunchbox::URI( "foo://*:" + portStr ));

    gotOne = false;
    gotTwo = false;
    for( size_t i = 0; i < 10; ++i )
    {
        BOOST_CHECK( publisher.publish(
                         zeq::vocabulary::serializeCamera( test::camera )));

        subscriber1.receive( 100 );
        if( gotOne && gotTwo )
            break;
    }
    BOOST_CHECK( gotOne );
    BOOST_CHECK( gotTwo );

    gotOne = false;
    gotTwo = false;
    for( size_t i = 0; i < 10; ++i )
    {
        BOOST_CHECK( publisher.publish(
                         zeq::vocabulary::serializeCamera( test::camera )));

        subscriber2.receive( 100 );
        if( gotOne && gotTwo )
            break;
    }
    BOOST_CHECK( gotOne );
    BOOST_CHECK( gotTwo );
}

BOOST_AUTO_TEST_CASE(test_publisher_routing)
{
    lunchbox::RNG rng;
    const unsigned short port = (rng.get<uint16_t>() % 60000) + 1024;
    const std::string& portStr = boost::lexical_cast< std::string >( port );
    zeq::Subscriber subscriber1( lunchbox::URI( "foo://localhost:1000" ));
    zeq::Subscriber subscriber2( lunchbox::URI( "foo://localhost:" + portStr ),
                                 subscriber1 );

    BOOST_CHECK( subscriber1.registerHandler( zeq::vocabulary::EVENT_CAMERA,
                                              boost::bind( &onEvent1, _1 )));
    BOOST_CHECK( subscriber2.registerHandler( zeq::vocabulary::EVENT_CAMERA,
                                              boost::bind( &onEvent2, _1 )));

    zeq::Publisher publisher( lunchbox::URI( "foo://*:" + portStr ));

    gotOne = false;
    gotTwo = false;
    for( size_t i = 0; i < 10; ++i )
    {
        BOOST_CHECK( publisher.publish(
                         zeq::vocabulary::serializeCamera( test::camera )));

        subscriber1.receive( 100 );
        if( gotTwo )
            break;
    }
    BOOST_CHECK( !gotOne );
    BOOST_CHECK( gotTwo );

    gotOne = false;
    gotTwo = false;
    for( size_t i = 0; i < 10; ++i )
    {
        BOOST_CHECK( publisher.publish(
                         zeq::vocabulary::serializeCamera( test::camera )));

        subscriber2.receive( 100 );
        if( gotTwo )
            break;
    }
    BOOST_CHECK( !gotOne );
    BOOST_CHECK( gotTwo );
}
