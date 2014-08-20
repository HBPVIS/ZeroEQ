
/* Copyright (c) 2014, Human Brain Project
 *                     Daniel Nachbaur <daniel.nachbaur@epfl.ch>
 */

#include <zeq/zeq.h>
#include <lunchbox/uri.h>

#define BOOST_TEST_MODULE zeq
#include <boost/test/unit_test.hpp>
#include <boost/bind.hpp>

namespace
{
const std::vector< float > camera( 16, 42 );

void onEvent( const zeq::Event& event )
{
    BOOST_CHECK( event.getType() == zeq::vocabulary::EVENT_CAMERA );
    const std::vector< float >& deserialized =
            zeq::vocabulary::deserializeCamera( event );
    BOOST_CHECK_EQUAL_COLLECTIONS( camera.begin(), camera.end(),
                                   deserialized.begin(), deserialized.end( ));
}
}

BOOST_AUTO_TEST_CASE(test_create_default_broker)
{
    zeq::Broker broker;
}

BOOST_AUTO_TEST_CASE(test_create_uri_broker)
{
    zeq::Broker broker( lunchbox::URI( "foo://" ));
}

BOOST_AUTO_TEST_CASE(test_create_invalid_uri_broker)
{
    // invalid URI, no hostname allowed
    BOOST_CHECK_THROW( zeq::Broker broker( lunchbox::URI( "foo://localhost" )),
                       std::runtime_error );
}

BOOST_AUTO_TEST_CASE(test_invalid_subscribe)
{
    zeq::Broker broker;
    BOOST_CHECK( !broker.subscribe( lunchbox::URI( "bar://*" )));

    BOOST_CHECK( broker.subscribe( lunchbox::URI( "foo://localhost:12345" )));
    BOOST_CHECK( !broker.subscribe( lunchbox::URI( "foo://localhost:12345" )));
}

BOOST_AUTO_TEST_CASE(test_valid_subscribe)
{
    zeq::Broker broker;
    BOOST_CHECK( broker.subscribe( lunchbox::URI( "foo://localhost:12345" )));
}

BOOST_AUTO_TEST_CASE(test_valid_unsubscribe)
{
    zeq::Broker broker;
    BOOST_CHECK( broker.subscribe( lunchbox::URI( "foo://localhost:12345" )));
    BOOST_CHECK( broker.unsubscribe( lunchbox::URI( "foo://localhost:12345" )));
}

BOOST_AUTO_TEST_CASE(test_invalid_unsubscribe)
{
    zeq::Broker broker;
    BOOST_CHECK( broker.subscribe( lunchbox::URI( "foo://localhost:12345" )));
    BOOST_CHECK( !broker.unsubscribe( lunchbox::URI( "foo://localhost:12346")));
}

BOOST_AUTO_TEST_CASE(test_invalid_publish)
{
    zeq::Broker broker;
    BOOST_CHECK_THROW(
                broker.publish( zeq::vocabulary::serializeCamera( camera )),
                std::runtime_error );
}

BOOST_AUTO_TEST_CASE(test_publish)
{
    zeq::Broker broker( lunchbox::URI( "foo://*:12345" ));
    BOOST_CHECK( broker.publish( zeq::vocabulary::serializeCamera( camera)));
}

BOOST_AUTO_TEST_CASE(test_registerhandler)
{
    zeq::Broker broker;
    BOOST_CHECK( broker.registerHandler( zeq::vocabulary::EVENT_CAMERA,
                                         boost::bind( &onEvent, _1 )));
}

BOOST_AUTO_TEST_CASE(test_deregisterhandler)
{
    zeq::Broker broker;
    BOOST_CHECK( broker.registerHandler( zeq::vocabulary::EVENT_CAMERA,
                                         boost::bind( &onEvent, _1 )));
    BOOST_CHECK( broker.deregisterHandler( zeq::vocabulary::EVENT_CAMERA ));
}

BOOST_AUTO_TEST_CASE(test_invalid_registerhandler)
{
    zeq::Broker broker;
    BOOST_CHECK( broker.registerHandler( zeq::vocabulary::EVENT_CAMERA,
                                         boost::bind( &onEvent, _1 )));
    BOOST_CHECK( !broker.registerHandler( zeq::vocabulary::EVENT_CAMERA,
                                          boost::bind( &onEvent, _1 )));
}

BOOST_AUTO_TEST_CASE(test_invalid_deregisterhandler)
{
    zeq::Broker broker;
    BOOST_CHECK( !broker.deregisterHandler( zeq::vocabulary::EVENT_CAMERA ));

    BOOST_CHECK( broker.registerHandler( zeq::vocabulary::EVENT_CAMERA,
                                         boost::bind( &onEvent, _1 )));
    BOOST_CHECK( !broker.deregisterHandler( zeq::vocabulary::EVENT_INVALID));
}

BOOST_AUTO_TEST_CASE(test_publish_receive)
{
    zeq::Broker subscriber;
    BOOST_CHECK(
               subscriber.subscribe( lunchbox::URI( "foo://localhost:12345" )));
    BOOST_CHECK( subscriber.registerHandler( zeq::vocabulary::EVENT_CAMERA,
                                             boost::bind( &onEvent, _1 )));

    zeq::Broker publisher( lunchbox::URI( "foo://*:12345" ));

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
    zeq::Broker publisher( lunchbox::URI( "foo://" ));

    zeq::Broker subscriber;
    BOOST_CHECK( subscriber.subscribe( lunchbox::URI( "foo://" )));
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
