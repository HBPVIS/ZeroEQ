
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

    for( size_t i = 0; i < 20; ++i )
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

BOOST_AUTO_TEST_CASE(two_subscribers)
{
    zeq::Publisher publisher( test::buildPublisherURI( ));
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

BOOST_AUTO_TEST_CASE(publisher_routing)
{
    zeq::Publisher publisher( test::buildPublisherURI( ));
    zeq::Subscriber* subscriber1 = new zeq::Subscriber(
                                      test::buildURI( "localhost" ));
    zeq::Subscriber subscriber2( test::buildURI( "localhost", publisher ),
                                 *subscriber1 );

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
