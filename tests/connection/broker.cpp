
/* Copyright (c) 2014-2015, Human Brain Project
 *                          Stefan.Eilemann@epfl.ch
 */

#define BOOST_TEST_MODULE zeq_connection_broker

#include "../broker.h"
#include <zeq/connection/broker.h>

#include <lunchbox/monitor.h>
#include <lunchbox/thread.h>
#include <lunchbox/servus.h>
#include <lunchbox/sleep.h>
#include <boost/bind.hpp>
#include <memory>

using boost::lexical_cast;

const unsigned short port = (lunchbox::RNG().get<uint16_t>() % 60000) + 1024;
const std::string brokerAddress = std::string( "127.0.0.1:" ) +
                                  lexical_cast< std::string >( port + 1 );
typedef std::unique_ptr< zeq::connection::Broker > BrokerPtr;

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
        BrokerPtr broker( createBroker( subscriber ));
        BOOST_CHECK( broker.get( ));
        if( !broker )
            return;

        running = true;
        for( size_t i = 0; i < 100 && !received ; ++i )
        {
            if( subscriber.receive( 100 ))
                received = true;
        }
        running = false;
    }

    bool received;
    lunchbox::Monitorb running;

protected:
    virtual BrokerPtr createBroker( zeq::Subscriber& subscriber )
    {
        return BrokerPtr(
            new zeq::connection::Broker( brokerAddress, subscriber ));
    }
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
    for( size_t i = 0; i < 100 && !subscriber.received; ++i )
    {
        BOOST_CHECK( publisher.publish(
                         zeq::vocabulary::serializeEcho( test::echoMessage )));
        lunchbox::sleep( 100 );
    }

    subscriber.join();
    BOOST_CHECK( subscriber.received );
}

template< zeq::connection::Broker::PortSelection mode >
class NamedSubscriber : public Subscriber
{
    BrokerPtr createBroker( zeq::Subscriber& subscriber ) override
    {
        // Multiple instances of the test may run concurrently. Try until we get
        // the well-defined port
        size_t nTries = 10;
        while( nTries-- )
        {
            try
            {
                return BrokerPtr(
                    new zeq::connection::Broker(
                      "zeq::connection::test_named_broker", subscriber, mode ));
                lunchbox::sleep( 100 );
            }
            catch( ... ) {}
        }
        return BrokerPtr();
    }
};

typedef NamedSubscriber< zeq::connection::Broker::PORT_FIXED >
    FixedNamedSubscriber;
typedef NamedSubscriber< zeq::connection::Broker::PORT_FIXED_OR_RANDOM >
    RandomNamedSubscriber;

BOOST_AUTO_TEST_CASE(test_named_broker)
{
    FixedNamedSubscriber subscriber1;
    subscriber1.start();
    RandomNamedSubscriber subscriber2;
    subscriber2.received = true;
    subscriber2.start();

    // Using a different scheme so zeroconf resolution does not work
    zeq::Publisher publisher( lunchbox::URI( "bar://*:" +
                                          lexical_cast< std::string >( port )));
    BOOST_CHECK( zeq::connection::Service::subscribe(
                     "127.0.0.1", "zeq::connection::test_named_broker",
                     publisher ));
    for( size_t i = 0; i < 100 && !subscriber1.received; ++i )
    {
        BOOST_CHECK( publisher.publish(
                         zeq::vocabulary::serializeEcho( test::echoMessage )));
        lunchbox::sleep( 100 );
    }

    subscriber2.join();
    subscriber1.join();
    BOOST_CHECK( subscriber1.received );
}

class FailingNamedSubscriber : public Subscriber
{
    BrokerPtr createBroker( zeq::Subscriber& subscriber ) override
    {
        BOOST_CHECK_THROW(
            new zeq::connection::Broker( "zeq::connection::test_named_broker",
                                         subscriber,
                                         zeq::connection::Broker::PORT_FIXED ),
            std::runtime_error );

        return BrokerPtr(
            new zeq::connection::Broker(
                "zeq::connection::test_named_broker", subscriber,
                zeq::connection::Broker::PORT_FIXED_OR_RANDOM ));
    }
};

BOOST_AUTO_TEST_CASE(test_named_broker_port_used)
{
    FixedNamedSubscriber subscriber1;
    subscriber1.start();
    subscriber1.running.waitEQ( true );

    FailingNamedSubscriber subscriber2;
    subscriber2.received = true;
    subscriber2.start();

    subscriber1.received = true;
    subscriber2.join();
    subscriber1.join();
}

BOOST_AUTO_TEST_CASE(test_invalid_broker)
{
    zeq::Subscriber subscriber( lunchbox::URI( "foo://127.0.0.1:" +
                                      lexical_cast< std::string >( port )));
    BOOST_CHECK_THROW( zeq::connection::Broker( std::string( "invalidIP" ),
                                                subscriber ),
                       std::runtime_error );
}
