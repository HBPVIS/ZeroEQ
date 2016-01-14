
/* Copyright (c) 2015-2016, Human Brain Project
 *                          Stefan.Eilemann@epfl.ch
 */

#define BOOST_TEST_MODULE zerobuf_receiver

#include "broker.h"

#include <zeq/detail/sender.h>

#include <servus/servus.h>

#include <chrono>

void testReceive( zeq::Publisher& publisher, zeq::Receiver& receiver,
                  test::EchoIn& var1, test::EchoIn& var2, const int line,
                  const bool notReceiveIsFailure = true )
{
    var1.gotData = false;
    var2.gotData = false;

    const auto startTime = std::chrono::high_resolution_clock::now();
    for( ;; )
    {
        BOOST_CHECK( publisher.publish(
                         zeq::vocabulary::Echo( test::echoMessage )));
        while( receiver.receive( 100 )) {}

        if( var1.gotData && var2.gotData )
            break;

        const auto endTime = std::chrono::high_resolution_clock::now();
        const auto elapsed =
            std::chrono::nanoseconds( endTime - startTime ).count() / 1000000;
        if( elapsed > 2000 /*ms*/ )
            break;
    }

    if( notReceiveIsFailure )
    {
        BOOST_CHECK_MESSAGE( var1.gotData,
                             "Event 1 not received (l." << line << ")"  );
        BOOST_CHECK_MESSAGE( var2.gotData,
                             "Event 2 not received (l." << line << ")"  );
    }
    else
        BOOST_CHECK( !var1.gotData && !var2.gotData );
}
void testReceive( zeq::Publisher& publisher, zeq::Receiver& receiver,
                  test::EchoIn& var, const int line,
                  const bool notReceiveIsFailure = true )
{
    testReceive( publisher, receiver, var, var, line, notReceiveIsFailure );
}

BOOST_AUTO_TEST_CASE(two_subscribers)
{
    zeq::Publisher publisher( zeq::NULL_SESSION );
    zeq::Subscriber subscriber1( test::buildURI( "localhost", publisher ));
    zeq::Subscriber subscriber2( test::buildURI( "localhost", publisher ),
                                 subscriber1 );
    test::EchoIn one;
    test::EchoIn two;
    BOOST_CHECK( subscriber1.subscribe( one ));
    BOOST_CHECK( subscriber2.subscribe( two ));

    testReceive( publisher, subscriber1, one, two, __LINE__ );
    testReceive( publisher, subscriber2, one, two, __LINE__ );
}

BOOST_AUTO_TEST_CASE(two_subscribers_zeroconf)
{
    if( !servus::Servus::isAvailable() || getenv("TRAVIS"))
        return;

    zeq::Publisher publisher( test::buildUniqueSession( ));
    zeq::detail::Sender::getUUID() = servus::make_UUID(); // different machine
    zeq::Subscriber subscriber1( publisher.getSession( ));
    zeq::Subscriber subscriber2( publisher.getSession(), subscriber1 );
    test::EchoIn one;
    test::EchoIn two;
    BOOST_CHECK( subscriber1.subscribe( one ));
    BOOST_CHECK( subscriber2.subscribe( two ));

    testReceive( publisher, subscriber1, one, two, __LINE__ );
    testReceive( publisher, subscriber2, one, two, __LINE__ );
}

BOOST_AUTO_TEST_CASE(publisher_routing)
{
    zeq::Publisher publisher( zeq::NULL_SESSION );
    zeq::Subscriber* subscriber1 = new zeq::Subscriber;
    zeq::Subscriber subscriber2( zeq::URI( publisher.getURI( )), *subscriber1 );

    test::EchoIn one;
    test::EchoIn two;
    BOOST_CHECK( subscriber1->subscribe( one ));
    BOOST_CHECK( subscriber2.subscribe( two ));

    testReceive( publisher, *subscriber1, two, __LINE__ );
    BOOST_CHECK( !one.gotData );

    testReceive( publisher, subscriber2, two, __LINE__ );
    BOOST_CHECK( !one.gotData );

    delete subscriber1;
    testReceive( publisher, subscriber2, two, __LINE__ );
    BOOST_CHECK( !one.gotData );
}

BOOST_AUTO_TEST_CASE(unsubscribe)
{
    zeq::Publisher publisher( zeq::NULL_SESSION );
    zeq::Subscriber subscriber( zeq::URI( publisher.getURI( )));

    test::EchoIn echo;
    BOOST_CHECK( subscriber.subscribe( echo ));
    BOOST_CHECK( !subscriber.subscribe( echo ));

    testReceive( publisher, subscriber, echo, __LINE__ );

    BOOST_CHECK( subscriber.unsubscribe( echo ));
    BOOST_CHECK( !subscriber.unsubscribe( echo ));

    testReceive( publisher, subscriber, echo, __LINE__, false );
}
