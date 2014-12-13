
/* Copyright (c) 2014, Human Brain Project
 *                     Daniel Nachbaur <daniel.nachbaur@epfl.ch>
 *                     Stefan.Eilemann@epfl.ch
 */

#define BOOST_TEST_MODULE zeq_pub_sub

#include "broker.h"

#include <lunchbox/sleep.h>
#include <lunchbox/thread.h>
#include <lunchbox/servus.h>

#include <boost/bind.hpp>

using namespace zeq::vocabulary;

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
            BOOST_CHECK(
                _publisher.publish( serializeEcho( test::echoMessage )));
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
    std::stringstream uri;
    uri << "foo://127.0.0.1:"
        << (lunchbox::RNG().get<uint16_t>() % 60000) + 1024;

    zeq::Publisher publisher( lunchbox::URI( "foo://" ));
    BOOST_CHECK_NO_THROW(
        zeq::Subscriber subscriber( lunchbox::URI( uri.str() )));
}

BOOST_AUTO_TEST_CASE(test_subscribe_to_different_schema)
{
    zeq::Publisher publisher( lunchbox::URI( "bar://" ));

    std::stringstream uriSubscriber;
    uriSubscriber << "bar://127.0.0.1:"
                  << (lunchbox::RNG().get<uint16_t>() % 60000) + 1024;

    BOOST_CHECK_NO_THROW(
        zeq::Subscriber subscriber( lunchbox::URI( uriSubscriber.str( ))));
}

BOOST_AUTO_TEST_CASE(test_subscribe_to_same_schema_zeroconf )
{
    if( !lunchbox::Servus::isAvailable() || getenv("TRAVIS"))
        return;

    zeq::Publisher publisher( lunchbox::URI( "foo://" ));
    BOOST_CHECK_NO_THROW(
        zeq::Subscriber subscriber( lunchbox::URI( "foo://" )));
}

BOOST_AUTO_TEST_CASE(test_subscribe_to_different_schema_zeroconf)
{
    if( !lunchbox::Servus::isAvailable() || getenv("TRAVIS"))
        return;

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
    BOOST_CHECK( subscriber.registerHandler(
                     EVENT_ECHO, boost::bind( &test::onEchoEvent, _1 )));

    zeq::Publisher publisher( lunchbox::URI( "foo://*:" + portStr ));

    bool received = false;
    for( size_t i = 0; i < 10; ++i )
    {
        BOOST_CHECK( publisher.publish( serializeEcho( test::echoMessage )));

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
    std::stringstream uri;
    uri << "foo://127.0.0.1:"
        << (lunchbox::RNG().get<uint16_t>() % 60000) + 1024;

    zeq::Subscriber subscriber( lunchbox::URI( uri.str() ));
    BOOST_CHECK( !subscriber.receive( 100 ));
}


BOOST_AUTO_TEST_CASE(test_no_receive_zeroconf)
{
    if( !lunchbox::Servus::isAvailable() || getenv("TRAVIS"))
        return;

    zeq::Subscriber subscriber( lunchbox::URI( "foo://" ));
    BOOST_CHECK( !subscriber.receive( 100 ));
}

BOOST_AUTO_TEST_CASE(test_publish_receive_zeroconf)
{
    if( !lunchbox::Servus::isAvailable() || getenv("TRAVIS"))
        return;

    zeq::Publisher publisher( lunchbox::URI( "foo://" ));
    zeq::Subscriber subscriber( lunchbox::URI( "foo://" ));

    BOOST_CHECK( subscriber.registerHandler(
                     EVENT_ECHO, boost::bind( &test::onEchoEvent, _1 )));

    bool received = false;
    for( size_t i = 0; i < 20; ++i )
    {
        BOOST_CHECK( publisher.publish(
                         zeq::vocabulary::serializeEcho( test::echoMessage )));

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
    if( !lunchbox::Servus::isAvailable() || getenv("TRAVIS"))
        return;

    zeq::Subscriber subscriber( lunchbox::URI( "foo://" ));
    BOOST_CHECK( subscriber.registerHandler(
                     EVENT_ECHO, boost::bind( &test::onEchoEvent, _1 )));

    Publisher publisher;
    publisher.start();
    BOOST_CHECK( subscriber.receive( ));

    publisher.running = false;
    BOOST_CHECK( publisher.join( ));
}

BOOST_AUTO_TEST_CASE(test_publish_receive_late_zeroconf)
{
    if( !lunchbox::Servus::isAvailable() || getenv("TRAVIS"))
        return;

    zeq::Subscriber subscriber( lunchbox::URI( "foo://" ));
    zeq::Publisher publisher( lunchbox::URI( "foo://" ));

    BOOST_CHECK( subscriber.registerHandler(
                     EVENT_ECHO, boost::bind( &test::onEchoEvent, _1 )));
    bool received = false;
    for( size_t i = 0; i < 20; ++i )
    {
        BOOST_CHECK( publisher.publish( serializeEcho( test::echoMessage )));

        if( subscriber.receive( 100 ))
        {
            received = true;
            break;
        }
    }
    BOOST_CHECK( received );
}


BOOST_AUTO_TEST_CASE(test_publish_receive_empty_event_zeroconf)
{
    if( !lunchbox::Servus::isAvailable() || getenv("TRAVIS"))
        return;

    zeq::Publisher publisher( lunchbox::URI( "foo://" ));
    zeq::Subscriber subscriber( lunchbox::URI( "foo://" ));

    BOOST_CHECK( subscriber.registerHandler(
                     EVENT_EXIT, boost::bind( &test::onExitEvent, _1 )));
    bool received = false;
    const zeq::Event event( EVENT_EXIT );
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
