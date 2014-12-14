
/* Copyright (c) 2014, Human Brain Project
 *                     Stefan.Eilemann@epfl.ch
 */

#define BOOST_TEST_MODULE zeq_receiver

#include "broker.h"

#include <lunchbox/sleep.h>
#include <lunchbox/thread.h>
#include <lunchbox/servus.h>
#include <boost/bind.hpp>

bool gotOne = false;
bool gotTwo = false;

void onEvent1( const zeq::Event& ) { gotOne = true; }
void onEvent2( const zeq::Event& ) { gotTwo = true; }

void testReceive( zeq::Publisher& publisher, zeq::Receiver& receiver,
                  bool& var1, bool& var2, const int line )
{
    using zeq::vocabulary::serializeEcho;
    gotOne = false;
    gotTwo = false;

    for( size_t i = 0; i < 10; ++i )
    {
        BOOST_CHECK( publisher.publish( serializeEcho( test::echoMessage )));
        receiver.receive( 100 );

        if( var1 && var2 )
            break;
    }
    BOOST_CHECK_MESSAGE( var1, (&var1 == &gotOne ? "Event 1" : "Event 2") <<
                               " not received (l." << line << ")" );
    if( &var1 != &var2 )
        BOOST_CHECK_MESSAGE( var2, (&var2 == &gotOne ? "Event 1" : "Event 2") <<
                                   " not received (l." << line << ")" );
}
void testReceive( zeq::Publisher& publisher, zeq::Receiver& receiver,
                  bool& var, const int line )
{
    testReceive( publisher, receiver, var, var, line );
}

BOOST_AUTO_TEST_CASE(test_two_subscribers)
{
    lunchbox::RNG rng;
    const unsigned short port = (rng.get<uint16_t>() % 60000) + 1024;
    const std::string& portStr = boost::lexical_cast< std::string >( port );
    zeq::Subscriber subscriber1( lunchbox::URI( "foo://localhost:" + portStr ));
    zeq::Subscriber subscriber2( lunchbox::URI( "foo://localhost:" + portStr ),
                                 subscriber1 );

    BOOST_CHECK( subscriber1.registerHandler( zeq::vocabulary::EVENT_ECHO,
                                              boost::bind( &onEvent1, _1 )));
    BOOST_CHECK( subscriber2.registerHandler( zeq::vocabulary::EVENT_ECHO,
                                              boost::bind( &onEvent2, _1 )));

    zeq::Publisher publisher( lunchbox::URI( "foo://*:" + portStr ));

    testReceive( publisher, subscriber1, gotOne, gotTwo, __LINE__ );
    testReceive( publisher, subscriber2, gotOne, gotTwo, __LINE__ );
}

BOOST_AUTO_TEST_CASE(test_publisher_routing)
{
    lunchbox::RNG rng;
    const unsigned short port = (rng.get<uint16_t>() % 60000) + 1024;
    const std::string& portStr = boost::lexical_cast< std::string >( port );
    zeq::Subscriber* subscriber1 =
        new zeq::Subscriber( lunchbox::URI( "foo://localhost:1000" ));
    zeq::Subscriber subscriber2( lunchbox::URI( "foo://localhost:" + portStr ),
                                 *subscriber1 );

    BOOST_CHECK( subscriber1->registerHandler( zeq::vocabulary::EVENT_ECHO,
                                               boost::bind( &onEvent1, _1 )));
    BOOST_CHECK( subscriber2.registerHandler( zeq::vocabulary::EVENT_ECHO,
                                              boost::bind( &onEvent2, _1 )));

    zeq::Publisher publisher( lunchbox::URI( "foo://*:" + portStr ));

    testReceive( publisher, *subscriber1, gotTwo, __LINE__ );
    BOOST_CHECK( !gotOne );

    testReceive( publisher, subscriber2, gotTwo, __LINE__ );
    BOOST_CHECK( !gotOne );

    delete subscriber1;
    testReceive( publisher, subscriber2, gotTwo, __LINE__ );
    BOOST_CHECK( !gotOne );
}
