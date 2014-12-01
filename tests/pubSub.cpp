
/* Copyright (c) 2014, Human Brain Project
 *                     Daniel Nachbaur <daniel.nachbaur@epfl.ch>
 *                     Stefan.Eilemann@epfl.ch
 */

#include "broker.h"

#include <lunchbox/sleep.h>
#include <lunchbox/thread.h>
#include <lunchbox/servus.h>
#include <boost/bind.hpp>

namespace
{
class Publisher : public lunchbox::Thread
{
public:
    Publisher()
        : running( true )
        , _publisher( lunchbox::URI( "foo://" ))
    {}

    void run() override
    {
        running = true;
        size_t i = 0;
        while( running )
        {
            BOOST_CHECK( _publisher.publish(
                             zeq::vocabulary::serializeCamera( test::camera )));
            lunchbox::sleep( 100 );
            ++i;

            if( i > 200 )
                LBTHROW( std::runtime_error( "Publisher giving up after 20s" ));
        }
    }

    bool running;

private:
    zeq::Publisher _publisher;
};
}

BOOST_AUTO_TEST_CASE(test_subscribe_to_same_schema)
{
    zeq::Publisher publisher( lunchbox::URI( "foo://" ));
    BOOST_CHECK_NO_THROW(
                zeq::Subscriber subscriber( lunchbox::URI( "foo://" )));
}

BOOST_AUTO_TEST_CASE(test_subscribe_to_different_schema)
{
    zeq::Publisher publisher( lunchbox::URI( "foo://" ));
    BOOST_CHECK_NO_THROW(
                zeq::Subscriber subscriber( lunchbox::URI( "bar://" )));
}

BOOST_AUTO_TEST_CASE(test_publish_receive)
{
    lunchbox::RNG rng;
    const unsigned short port = (rng.get<uint16_t>() % 60000) + 1024;
    const std::string& portStr = boost::lexical_cast< std::string >( port );
    zeq::Subscriber subscriber( lunchbox::URI( "foo://localhost:" + portStr ));
    BOOST_CHECK( subscriber.registerHandler( zeq::vocabulary::EVENT_CAMERA,
                                      boost::bind( &test::onCameraEvent, _1 )));

    zeq::Publisher publisher( lunchbox::URI( "foo://*:" + portStr ));

    bool received = false;
    for( size_t i = 0; i < 10; ++i )
    {
        BOOST_CHECK( publisher.publish(
                         zeq::vocabulary::serializeCamera( test::camera )));

        if( subscriber.receive( 100 ))
        {
            received = true;
            break;
        }
    }
    BOOST_CHECK( received );
}

BOOST_AUTO_TEST_CASE(test_no_receive)
{
    zeq::Subscriber subscriber( lunchbox::URI( "foo://" ));
    BOOST_CHECK( !subscriber.receive( 100 ));
}

BOOST_AUTO_TEST_CASE(test_publish_receive_zeroconf)
{
    if( !lunchbox::Servus::isAvailable( ) )
        return;

    zeq::Publisher publisher( lunchbox::URI( "foo://" ));
    zeq::Subscriber subscriber( lunchbox::URI( "foo://" ));

    BOOST_CHECK( subscriber.registerHandler( zeq::vocabulary::EVENT_CAMERA,
                                      boost::bind( &test::onCameraEvent, _1 )));

    bool received = false;
    for( size_t i = 0; i < 20; ++i )
    {
        BOOST_CHECK( publisher.publish(
                         zeq::vocabulary::serializeCamera( test::camera )));

        if( subscriber.receive( 100 ))
        {
            received = true;
            break;
        }
    }
    BOOST_CHECK( received );
}

BOOST_AUTO_TEST_CASE(test_publish_blocking_receive_zeroconf)
{
    if( !lunchbox::Servus::isAvailable( ) )
        return;

    zeq::Subscriber subscriber( lunchbox::URI( "foo://" ));
    BOOST_CHECK( subscriber.registerHandler( zeq::vocabulary::EVENT_CAMERA,
                                      boost::bind( &test::onCameraEvent, _1 )));

    Publisher publisher;
    publisher.start();
    BOOST_CHECK( subscriber.receive( ));

    publisher.running = false;
    BOOST_CHECK( publisher.join( ));
}

BOOST_AUTO_TEST_CASE(test_publish_receive_late_zeroconf)
{
    if( !lunchbox::Servus::isAvailable( ) )
        return;

    zeq::Subscriber subscriber( lunchbox::URI( "foo://" ));
    zeq::Publisher publisher( lunchbox::URI( "foo://" ));

    BOOST_CHECK( subscriber.registerHandler( zeq::vocabulary::EVENT_CAMERA,
                                      boost::bind( &test::onCameraEvent, _1 )));
    bool received = false;
    for( size_t i = 0; i < 20; ++i )
    {
        BOOST_CHECK( publisher.publish(
                         zeq::vocabulary::serializeCamera( test::camera )));

        if( subscriber.receive( 100 ))
        {
            received = true;
            break;
        }
    }
    BOOST_CHECK( received );
}

BOOST_AUTO_TEST_CASE(test_publish_receive_empty_event)
{
    if( !lunchbox::Servus::isAvailable( ) )
        return;

    zeq::Publisher publisher( lunchbox::URI( "foo://" ));
    zeq::Subscriber subscriber( lunchbox::URI( "foo://" ));

    BOOST_CHECK( subscriber.registerHandler( zeq::vocabulary::EVENT_EXIT,
                                        boost::bind( &test::onExitEvent, _1 )));
    bool received = false;
    const zeq::Event event( zeq::vocabulary::EVENT_EXIT );
    for( size_t i = 0; i < 20; ++i )
    {
        BOOST_CHECK( publisher.publish( event ));

        if( subscriber.receive( 100 ))
        {
            received = true;
            break;
        }
    }
    BOOST_CHECK( received );
}
