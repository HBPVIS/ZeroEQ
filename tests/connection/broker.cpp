
/* Copyright (c) 2014-2015, Human Brain Project
 *                          Stefan.Eilemann@epfl.ch
 *                          Juan Hernando <jhernando@fi.upm.es>
 */

#define BOOST_TEST_MODULE zeq_connection_broker

#include "../broker.h"
#include <zeq/connection/broker.h>

#include <servus/servus.h>

#include <condition_variable>
#include <memory>
#include <mutex>
#include <thread>

const unsigned int port = zeq::detail::getRandomPort();
const std::string brokerAddress = std::string( "127.0.0.1:" ) +
                                  std::to_string( port + 1 );
typedef std::unique_ptr< zeq::connection::Broker > BrokerPtr;

class Subscriber
{
public:
    Subscriber()
        : received( false )
        , _started( false )
    {}

    virtual ~Subscriber() {}

    void run()
    {
        zeq::Subscriber subscriber( test::buildURI( "foo", "127.0.0.1", port ));
#ifdef ZEQ_USE_ZEROBUF
        test::EchoIn echo;
        BOOST_CHECK( subscriber.subscribe( echo ));
#endif
        BOOST_CHECK( subscriber.registerHandler( zeq::vocabulary::EVENT_ECHO,
           std::bind( &Subscriber::onEchoEvent, this, std::placeholders::_1 )));

        // Using the connection broker in place of zeroconf
        BrokerPtr broker( createBroker( subscriber ));
        BOOST_CHECK( broker.get( ));
        if( !broker )
            return;

        {
            std::unique_lock< std::mutex > lock( _mutex );
            _started = true;
            _condition.notify_all();
        }

        // test receive of data for echo event (flatbuffers, 'received') and
        // echo object (zerobuf, 'echo.gotData')
        for( size_t i = 0; i < 100 && !received ; ++i )
        {
            if( subscriber.receive( 100 ))
            {
#ifdef ZEQ_USE_ZEROBUF
                received = echo.gotData && received;
#endif
            }
        }
    }

    void waitStarted() const
    {
        std::unique_lock< std::mutex > lock( _mutex );
        while( !_started )
            _condition.wait( lock );
    }

    bool received;

protected:
    mutable std::condition_variable _condition;
    mutable std::mutex _mutex;
    bool _started;

    void onEchoEvent( const zeq::Event& event )
    {
        test::onEchoEvent( event );
        received = true;
    }

    virtual BrokerPtr createBroker( zeq::Subscriber& subscriber )
    {
        return BrokerPtr(
            new zeq::connection::Broker( brokerAddress, subscriber ));
    }
};

BOOST_AUTO_TEST_CASE(test_broker)
{
    Subscriber subscriber;
    std::thread thread( std::bind( &Subscriber::run, &subscriber ));

    // Using a different scheme so zeroconf resolution does not work
    zeq::Publisher publisher( test::buildURI( "bar", "*", port ));
    BOOST_CHECK( zeq::connection::Service::subscribe( brokerAddress,
                                                      publisher ));
#ifdef ZEQ_USE_ZEROBUF
    const test::EchoOut echo;
#endif
    for( size_t i = 0; i < 100 && !subscriber.received; ++i )
    {
#ifdef ZEQ_USE_ZEROBUF
        BOOST_CHECK( publisher.publish( echo ));
#endif
        BOOST_CHECK( publisher.publish(
                         zeq::vocabulary::serializeEcho( test::echoMessage )));
        std::this_thread::sleep_for( std::chrono::milliseconds( 100 ));
    }

    thread.join();
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
            }
            catch( ... ) {}

            std::this_thread::sleep_for( std::chrono::milliseconds( 100 ));
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
    std::thread thread1( std::bind( &Subscriber::run, &subscriber1 ));

    RandomNamedSubscriber subscriber2;
    subscriber2.received = true;
    std::thread thread2( std::bind( &Subscriber::run, &subscriber2 ));

    // Using a different scheme so zeroconf resolution does not work
    zeq::Publisher publisher( test::buildURI( "bar", "*", port ));
    BOOST_CHECK( zeq::connection::Service::subscribe(
                     "127.0.0.1", "zeq::connection::test_named_broker",
                     publisher ));

#ifdef ZEQ_USE_ZEROBUF
    const test::EchoOut echo;
#endif
    for( size_t i = 0; i < 100 && !subscriber1.received; ++i )
    {
#ifdef ZEQ_USE_ZEROBUF
        BOOST_CHECK( publisher.publish( echo ));
#endif
        BOOST_CHECK( publisher.publish(
                         zeq::vocabulary::serializeEcho( test::echoMessage )));
        std::this_thread::sleep_for( std::chrono::milliseconds( 100 ));
    }

    thread2.join();
    thread1.join();
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

    std::thread thread1( std::bind( &Subscriber::run, &subscriber1 ));

    subscriber1.waitStarted();

    FailingNamedSubscriber subscriber2;
    subscriber2.received = true;
    std::thread thread2( std::bind( &Subscriber::run, &subscriber2 ));

    subscriber1.received = true;
    thread2.join();
    thread1.join();
}

BOOST_AUTO_TEST_CASE(test_invalid_broker)
{
    zeq::Subscriber subscriber( test::buildURI( "foo", "127.0.0.1", port ));
    BOOST_CHECK_THROW( zeq::connection::Broker( std::string( "invalidIP" ),
                                                subscriber ),
                       std::runtime_error );
}
