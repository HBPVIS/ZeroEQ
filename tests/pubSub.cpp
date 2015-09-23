
/* Copyright (c) 2014-2015, Human Brain Project
 *                          Daniel Nachbaur <daniel.nachbaur@epfl.ch>
 *                          Stefan.Eilemann@epfl.ch
 */

#define BOOST_TEST_MODULE zeq_pub_sub

#include "broker.h"

#include <servus/servus.h>

#include <thread>
#include <chrono>

using namespace zeq::vocabulary;

BOOST_AUTO_TEST_CASE(subscribe_to_same_schema)
{
    zeq::Publisher publisher( test::buildPublisherURI( ));
    BOOST_CHECK_NO_THROW( zeq::Subscriber subscriber(
                              test::buildURI( "localhost" )));
}

BOOST_AUTO_TEST_CASE(subscribe_to_different_schema)
{
    zeq::Publisher publisher( test::buildPublisherURI( ));
    zeq::URI uri = test::buildURI( "localhost" );
    uri.setScheme( uri.getScheme() + "bar" );
    BOOST_CHECK_NO_THROW( zeq::Subscriber subscriber( uri ));
}

BOOST_AUTO_TEST_CASE(publish_receive)
{
    zeq::Publisher publisher( test::buildURI( "*" ));
    const servus::URI uri = test::buildURI( "localhost", publisher );
    zeq::Subscriber subscriber( uri );
    BOOST_CHECK( subscriber.registerHandler( EVENT_ECHO,
                       std::bind( &test::onEchoEvent, std::placeholders::_1 )));


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

BOOST_AUTO_TEST_CASE(no_receive)
{
    zeq::Subscriber subscriber( test::buildURI( "127.0.0.1" ));
    BOOST_CHECK( !subscriber.receive( 100 ));
}

BOOST_AUTO_TEST_CASE( subscribe_to_same_schema_zeroconf )
{
    if( !servus::Servus::isAvailable( ))
        return;

    zeq::Publisher publisher( test::buildPublisherURI( ));
    BOOST_CHECK_NO_THROW(
        zeq::Subscriber subscriber( test::buildPublisherURI( )));
}

BOOST_AUTO_TEST_CASE(subscribe_to_different_schema_zeroconf)
{
    if( !servus::Servus::isAvailable() || getenv("TRAVIS"))
        return;

    zeq::URI uri = test::buildPublisherURI();
    zeq::Publisher publisher( uri );
    uri.setScheme( uri.getScheme() + " bar" );
    BOOST_CHECK_NO_THROW( zeq::Subscriber subscriber( uri ));
}

BOOST_AUTO_TEST_CASE(no_receive_zeroconf)
{
    if( !servus::Servus::isAvailable() || getenv("TRAVIS"))
        return;

    zeq::Subscriber subscriber( test::buildPublisherURI( ));
    BOOST_CHECK( !subscriber.receive( 100 ));
}

BOOST_AUTO_TEST_CASE(publish_receive_zeroconf)
{
    if( !servus::Servus::isAvailable() || getenv("TRAVIS"))
        return;

    zeq::URI uri = test::buildPublisherURI();
    zeq::Publisher publisher( uri );
    zeq::Subscriber noSubscriber( uri );

    uri.addQuery( "subscribeSelf", "true" );
    zeq::Subscriber subscriber( uri );

    BOOST_CHECK( subscriber.registerHandler( EVENT_ECHO,
                       std::bind( &test::onEchoEvent, std::placeholders::_1 )));
    BOOST_CHECK( noSubscriber.registerHandler( EVENT_ECHO,
                       std::bind( &test::onEchoEvent, std::placeholders::_1 )));

    bool received = false;
    for( size_t i = 0; i < 20; ++i )
    {
        BOOST_CHECK( publisher.publish(
                         zeq::vocabulary::serializeEcho( test::echoMessage )));

        BOOST_CHECK( !noSubscriber.receive( 100 ));
        if( subscriber.receive( 0 ))
        {
            received = true;
            break;
        }
    }
    BOOST_CHECK( received );
}

BOOST_AUTO_TEST_CASE(publish_receive_zeroconf_disabled)
{
    if( getenv("TRAVIS"))
        return;

    zeq::Publisher publisher( test::buildPublisherURI(), zeq::ANNOUNCE_NONE );
    zeq::Subscriber subscriber( test::buildPublisherURI( ));

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

void onLargeEcho( const zeq::Event& event )
{
    BOOST_CHECK( event.getType() == zeq::vocabulary::EVENT_ECHO );
}

BOOST_AUTO_TEST_CASE(publish_receive_filters)
{
    // The publisher needs to be destroyed before the subscriber otherwise
    // zmq_ctx_destroy() can hang forever. For more details see
    // zmq_ctx_destroy() documentation.
    zeq::Publisher* publisher = new zeq::Publisher( test::buildPublisherURI(),
                                                    zeq::ANNOUNCE_NONE );
    zeq::Subscriber subscriber( publisher->getURI( ));
    const std::string message( 60000, 'a' );

    // Make sure we're connected
    BOOST_CHECK( subscriber.registerHandler( EVENT_ECHO,
                       std::bind( &test::onEchoEvent, std::placeholders::_1 )));
    for( size_t i = 0; i < 20; ++i )
    {
        BOOST_CHECK( publisher->publish(
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
        BOOST_CHECK( publisher->publish( event ));
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
        BOOST_CHECK( publisher->publish( event ));
        while( subscriber.receive( 0 )) /* NOP to drain */;
    }

    const auto& echoTime = std::chrono::high_resolution_clock::now() -
                           startTime;

    BOOST_CHECK_MESSAGE( noEchoTime < echoTime,
                         std::chrono::nanoseconds( noEchoTime ).count() << ", "
                         << std::chrono::nanoseconds( echoTime ).count( ));
    delete publisher;
}

BOOST_AUTO_TEST_CASE(publish_receive_late_zeroconf)
{
    if( !servus::Servus::isAvailable() || getenv("TRAVIS"))
        return;

    zeq::URI uri = test::buildPublisherURI();
    uri.addQuery( "subscribeSelf", "true" );
    zeq::Subscriber subscriber( uri );
    zeq::Publisher publisher( test::buildPublisherURI( ));

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

BOOST_AUTO_TEST_CASE(publish_receive_empty_event_zeroconf)
{
    if( !servus::Servus::isAvailable() || getenv("TRAVIS"))
        return;

    zeq::URI uri = test::buildPublisherURI();
    zeq::Publisher publisher( uri );

    uri.addQuery( "subscribeSelf", "true" );
    zeq::Subscriber subscriber( uri );

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

namespace
{
class Publisher
{
public:
    Publisher()
        : running( true )
        , _publisher( test::buildPublisherURI( ))
    {}

    void run()
    {
        running = true;
        size_t i = 0;
        while( running )
        {
            BOOST_CHECK(
                _publisher.publish( serializeEcho( test::echoMessage )));
            std::this_thread::sleep_for( std::chrono::milliseconds( 100 ));
            ++i;

            if( i > 200 )
                ZEQTHROW( std::runtime_error( "Publisher giving up after 20s"));
        }
    }

    bool running;

private:
    zeq::Publisher _publisher;
};
}


BOOST_AUTO_TEST_CASE(publish_blocking_receive_zeroconf)
{
    if( !servus::Servus::isAvailable() || getenv("TRAVIS"))
        return;

    zeq::URI uri = test::buildPublisherURI();
    uri.addQuery( "subscribeSelf", "true" );
    zeq::Subscriber subscriber( uri );

    BOOST_CHECK( subscriber.registerHandler( EVENT_ECHO,
                       std::bind( &test::onEchoEvent, std::placeholders::_1 )));

    Publisher publisher;
    std::thread thread( std::bind( &Publisher::run, &publisher ));
    BOOST_CHECK( subscriber.receive( ));

    publisher.running = false;
    thread.join( );
}

#ifdef ZEQ_USE_ZEROBUF
BOOST_AUTO_TEST_CASE(publish_receive_zerobuf)
{
    zeq::vocabulary::Echo echoOut;
    zeq::vocabulary::Echo echoIn;

    echoOut.setMessage( "The quick brown fox" );

    zeq::Publisher publisher( test::buildURI( "*" ));
    const servus::URI uri = test::buildURI( "localhost", publisher );
    zeq::Subscriber subscriber( uri );
    BOOST_CHECK( subscriber.subscribe( echoIn ));

    for( size_t i = 0; i < 10; ++i )
    {
        BOOST_CHECK( publisher.publish( echoOut ));

        if( subscriber.receive( 100 ))
        {
            BOOST_CHECK_EQUAL( echoIn.getMessageString(),
                               echoOut.getMessageString( ));
            return;
        }
    }
    BOOST_CHECK( !"reachable" );
}
#endif
