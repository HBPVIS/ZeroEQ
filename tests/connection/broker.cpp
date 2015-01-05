
/* Copyright (c) 2014-2015, Human Brain Project
 *                          Stefan.Eilemann@epfl.ch
 */

#define BOOST_TEST_MODULE zeq_connection_broker

#include "../broker.h"
#include <zeq/connection/broker.h>

#include <lunchbox/sleep.h>
#include <lunchbox/thread.h>
#include <lunchbox/servus.h>
#include <boost/bind.hpp>

using boost::lexical_cast;

const unsigned short port = (lunchbox::RNG().get<uint16_t>() % 60000) + 1024;
const std::string brokerAddress = std::string( "127.0.0.1:" ) +
                                  lexical_cast< std::string >( port + 1 );

class Subscriber : public lunchbox::Thread
{
public:
    Subscriber() : received( false ) {}

    void run() final
    {
        zeq::Subscriber subscriber( lunchbox::URI( "foo://127.0.0.1:" +
                                          lexical_cast< std::string >( port )));
        BOOST_CHECK( subscriber.registerHandler( zeq::vocabulary::EVENT_ECHO,
                                        boost::bind( &test::onEchoEvent, _1 )));

        // Using the connection broker in place of zeroconf
        zeq::connection::Broker broker( brokerAddress, subscriber );

        for( size_t i = 0; i < 10; ++i )
        {
            if( subscriber.receive( 100 ))
            {
                received = true;
                return;
            }
        }
    }

    bool received;
};

BOOST_AUTO_TEST_CASE(test_broker)
{
    Subscriber subscriber;
    subscriber.start();

    // Using a different scheme so zeroconf resolution does not work
    zeq::Publisher publisher( lunchbox::URI( "bar://*:" +
                                          lexical_cast< std::string >( port )));
    BOOST_CHECK( zeq::connection::Service::subscribe( brokerAddress,
                                                      publisher ));
    for( size_t i = 0; i < 10 && !subscriber.received; ++i )
    {
        BOOST_CHECK( publisher.publish(
                         zeq::vocabulary::serializeEcho( test::echoMessage )));
        lunchbox::sleep( 100 );
    }

    subscriber.join();
    BOOST_CHECK( subscriber.received );
}
