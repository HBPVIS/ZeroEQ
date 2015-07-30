
/* Copyright (c) 2015, Human Brain Project
 *                     Stefan.Eilemann@epfl.ch
 */

#define BOOST_TEST_MODULE zerobuf_receiver

#include "broker.h"

void testReceive( zeq::Publisher& publisher, zeq::Receiver& receiver,
                  test::EchoIn& var1, test::EchoIn& var2, const int line )
{
    var1.gotData = false;
    var2.gotData = false;

    for( size_t i = 0; i < 10; ++i )
    {
        BOOST_CHECK( publisher.publish( test::EchoOut( )));
        receiver.receive( 100 );

        if( var1.gotData && var2.gotData )
            break;
    }
    BOOST_CHECK_MESSAGE( var1.gotData,
                         "Event 1 not received (l." << line << ")"  );
    BOOST_CHECK_MESSAGE( var2.gotData,
                         "Event 2 not received (l." << line << ")"  );
}
void testReceive( zeq::Publisher& publisher, zeq::Receiver& receiver,
                  test::EchoIn& var, const int line )
{
    testReceive( publisher, receiver, var, var, line );
}

BOOST_AUTO_TEST_CASE(test_two_subscribers)
{
    const unsigned short port = zeq::detail::getRandomPort();
    zeq::Subscriber subscriber1( test::buildURI( "foo", "localhost", port ));
    zeq::Subscriber subscriber2( test::buildURI( "foo", "localhost", port ),
                                 subscriber1 );
    test::EchoIn one;
    test::EchoIn two;
    BOOST_CHECK( subscriber1.subscribe( one ));
    BOOST_CHECK( subscriber2.subscribe( two ));

    zeq::Publisher publisher( test::buildPublisherURI( "foo", port ));

    testReceive( publisher, subscriber1, one, two, __LINE__ );
    testReceive( publisher, subscriber2, one, two, __LINE__ );
}

BOOST_AUTO_TEST_CASE(test_publisher_routing)
{
    const unsigned short port = zeq::detail::getRandomPort();
    zeq::Subscriber* subscriber1 =
        new zeq::Subscriber( test::buildURI( "foo", "localhost", 1000 ));
    zeq::Subscriber subscriber2( test::buildURI( "foo", "localhost", port ),
                                 *subscriber1 );

    test::EchoIn one;
    test::EchoIn two;
    BOOST_CHECK( subscriber1->subscribe( one ));
    BOOST_CHECK( subscriber2.subscribe( two ));

    zeq::Publisher publisher( test::buildPublisherURI( "foo", port ));

    testReceive( publisher, *subscriber1, two, __LINE__ );
    BOOST_CHECK( !one.gotData );

    testReceive( publisher, subscriber2, two, __LINE__ );
    BOOST_CHECK( !one.gotData );

    delete subscriber1;
    testReceive( publisher, subscriber2, two, __LINE__ );
    BOOST_CHECK( !one.gotData );
}
