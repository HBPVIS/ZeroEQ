
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

typedef std::unique_ptr< zeq::connection::Broker > BrokerPtr;
std::string _broker;
zeq::Publisher* _publisher = 0;

class Subscriber
{
public:
    Subscriber()
        : received( false )
        , _state( STATE_CREATED )
    {}

    virtual ~Subscriber() {}

    void run()
    {
        zeq::Subscriber subscriber( test::buildURI( "127.0.0.1", *_publisher ));
#ifdef ZEQ_USE_ZEROBUF
        test::EchoIn echo;
        BOOST_CHECK( subscriber.subscribe( echo ));
#endif
        BOOST_CHECK( subscriber.registerHandler( zeq::vocabulary::EVENT_ECHO,
           std::bind( &Subscriber::onEchoEvent, this, std::placeholders::_1 )));

        // Using the connection broker in place of zeroconf
        BrokerPtr broker = createBroker( subscriber );
        BOOST_REQUIRE( broker.get( ));
        if( !broker )
            return;

        _broker = broker->getAddress();
        {
            std::unique_lock< std::mutex > lock( _mutex );
            _state = STATE_STARTED;
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
        while( _state < STATE_STARTED )
            _condition.wait( lock );
    }

    void setRun()
    {
        std::unique_lock< std::mutex > lock( _mutex );
        _state = STATE_RUN;
        _condition.notify_all();
    }

    void waitRun() const
    {
        std::unique_lock< std::mutex > lock( _mutex );
        while( _state < STATE_RUN )
            _condition.wait( lock );
    }

    bool received;

protected:
    mutable std::condition_variable _condition;
    mutable std::mutex _mutex;
    enum State
    {
        STATE_CREATED,
        STATE_STARTED,
        STATE_RUN
    } _state;

    void onEchoEvent( const zeq::Event& event )
    {
        test::onEchoEvent( event );
        received = true;
    }

    virtual BrokerPtr createBroker( zeq::Subscriber& subscriber )
    {
        return BrokerPtr(
            new zeq::connection::Broker( "127.0.0.1:0", subscriber ));
    }
};

BOOST_AUTO_TEST_CASE( broker )
{
    // Using a different scheme so zeroconf resolution does not work
    zeq::Publisher publisher( test::buildURI( "*" ));
    _publisher = &publisher;
    _broker.clear();

    Subscriber subscriber;
    std::thread thread( std::bind( &Subscriber::run, &subscriber ));
    subscriber.waitStarted();

    BOOST_CHECK( zeq::connection::Service::subscribe( _broker, publisher ));
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
    _publisher = 0;
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

            waitRun();
        }
        return BrokerPtr();
    }
};

typedef NamedSubscriber< zeq::connection::Broker::PORT_FIXED >
    FixedNamedSubscriber;
typedef NamedSubscriber< zeq::connection::Broker::PORT_FIXED_OR_RANDOM >
    RandomNamedSubscriber;

BOOST_AUTO_TEST_CASE( named_broker )
{
    // Using a different scheme so zeroconf resolution does not work
    zeq::URI uri = test::buildURI( "*" );
    uri.setScheme( "foo" );
    zeq::Publisher publisher( uri );
    _publisher = &publisher;

    FixedNamedSubscriber subscriber1;
    std::thread thread1( std::bind( &Subscriber::run, &subscriber1 ));
    subscriber1.waitStarted();

    RandomNamedSubscriber subscriber2;
    subscriber2.received = true;
    std::thread thread2( std::bind( &Subscriber::run, &subscriber2 ));

    subscriber1.setRun();
    subscriber2.setRun();

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
    _publisher = 0;
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

BOOST_AUTO_TEST_CASE( named_broker_port_used )
{
    if( getenv( "TRAVIS" ))
        return;

    zeq::Publisher publisher( test::buildURI( "*" ));
    _publisher = &publisher;

    FixedNamedSubscriber subscriber1;
    std::thread thread1( std::bind( &Subscriber::run, &subscriber1 ));
    subscriber1.waitStarted();

    FailingNamedSubscriber subscriber2;
    subscriber2.received = true;
    std::thread thread2( std::bind( &Subscriber::run, &subscriber2 ));
    subscriber2.waitStarted();

    subscriber1.received = true;
    subscriber1.setRun();
    thread2.join();
    thread1.join();

    _publisher = 0;
}

BOOST_AUTO_TEST_CASE( invalid_broker )
{
    zeq::Subscriber subscriber( test::buildURI( "127.0.0.1" ));
    BOOST_CHECK_THROW( zeq::connection::Broker( std::string( "invalidIP" ),
                                                subscriber ),
                       std::runtime_error );
}
