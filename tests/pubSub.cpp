
/* Copyright (c) 2014-2015, Human Brain Project
 *                          Daniel Nachbaur <daniel.nachbaur@epfl.ch>
 *                          Stefan.Eilemann@epfl.ch
 */

#define BOOST_TEST_MODULE zeq_pub_sub

#include "broker.h"

#include <lunchbox/servus.h>
#include <lunchbox/sleep.h>
#include <lunchbox/thread.h>
#include <chrono>

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

BOOST_AUTO_TEST_CASE(test_subscribe_to_same_schema_zeroconf)
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
    const std::string& portStr = std::to_string( ( unsigned int ) port );
    zeq::Subscriber subscriber( lunchbox::URI( "foo://localhost:" + portStr ));
    BOOST_CHECK( subscriber.registerHandler( EVENT_ECHO,
                       std::bind( &test::onEchoEvent, std::placeholders::_1 )));

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

    BOOST_CHECK( subscriber.registerHandler( EVENT_ECHO,
                       std::bind( &test::onEchoEvent, std::placeholders::_1 )));

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

BOOST_AUTO_TEST_CASE(test_publish_receive_zeroconf_disabled)
{
    if( getenv("TRAVIS"))
        return;

    zeq::Publisher publisher( lunchbox::URI( "foo://" ), zeq::ANNOUNCE_NONE );
    zeq::Subscriber subscriber( lunchbox::URI( "foo://" ));

    BOOST_CHECK( subscriber.registerHandler(
                     EVENT_ECHO, std::bind( &test::onEchoEvent,
                                            std::placeholders::_1 )));

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
    BOOST_CHECK( !received );
}

BOOST_AUTO_TEST_CASE(test_publish_blocking_receive_zeroconf)
{
    if( !lunchbox::Servus::isAvailable() || getenv("TRAVIS"))
        return;

    zeq::Subscriber subscriber( lunchbox::URI( "foo://" ));
    BOOST_CHECK( subscriber.registerHandler( EVENT_ECHO,
                       std::bind( &test::onEchoEvent, std::placeholders::_1 )));

    Publisher publisher;
    publisher.start();
    BOOST_CHECK( subscriber.receive( ));

    publisher.running = false;
    BOOST_CHECK( publisher.join( ));
}

void onLargeEcho( const zeq::Event& event )
{
    BOOST_CHECK( event.getType() == zeq::vocabulary::EVENT_ECHO );
}

BOOST_AUTO_TEST_CASE(test_publish_receive_filters)
{
    zeq::Publisher publisher( lunchbox::URI( "foo://" ), zeq::ANNOUNCE_NONE );
    zeq::Subscriber subscriber( publisher.getURI( ));
    const std::string message( 60000, 'a' );

    // Make sure we're connected
    BOOST_CHECK( subscriber.registerHandler( EVENT_ECHO,
                       std::bind( &test::onEchoEvent, std::placeholders::_1 )));
    for( size_t i = 0; i < 20; ++i )
    {
        BOOST_CHECK( publisher.publish(
                         zeq::vocabulary::serializeEcho( test::echoMessage )));
        if( subscriber.receive( 100 ))
            break;
    }
    BOOST_CHECK( subscriber.deregisterHandler( EVENT_ECHO ));

    // benchmark with no data to be transmitted
    const zeq::Event& event = serializeEcho( message );
    auto startTime = std::chrono::high_resolution_clock::now();
    for( size_t i = 0; i < 1000; ++i )
    {
        BOOST_CHECK( publisher.publish( event ));
        while( subscriber.receive( 0 )) /* NOP to drain */;
    }
    const auto& noEchoTime = std::chrono::high_resolution_clock::now() -
                             startTime;

    // Benchmark with echo handler, now should send data
    BOOST_CHECK( subscriber.registerHandler( EVENT_ECHO,
                       std::bind( &onLargeEcho, std::placeholders::_1 )));

    startTime = std::chrono::high_resolution_clock::now();
    for( size_t i = 0; i < 1000; ++i )
    {
        BOOST_CHECK( publisher.publish( event ));
        while( subscriber.receive( 0 )) /* NOP to drain */;
    }
    const auto& echoTime = std::chrono::high_resolution_clock::now() -
                           startTime;

    BOOST_CHECK_MESSAGE( noEchoTime < echoTime,
                         std::chrono::nanoseconds( noEchoTime ).count() << ", "
                         << std::chrono::nanoseconds( echoTime ).count( ));
}

BOOST_AUTO_TEST_CASE(test_publish_receive_late_zeroconf)
{
    if( !lunchbox::Servus::isAvailable() || getenv("TRAVIS"))
        return;

    zeq::Subscriber subscriber( lunchbox::URI( "foo://" ));
    zeq::Publisher publisher( lunchbox::URI( "foo://" ));

    BOOST_CHECK( subscriber.registerHandler( EVENT_ECHO,
                       std::bind( &test::onEchoEvent, std::placeholders::_1 )));
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

    BOOST_CHECK( subscriber.registerHandler( EVENT_EXIT,
                       std::bind( &test::onExitEvent, std::placeholders::_1 )));
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
