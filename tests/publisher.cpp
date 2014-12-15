
/* Copyright (c) 2014, Human Brain Project
 *                     Daniel Nachbaur <daniel.nachbaur@epfl.ch>
 *                     Stefan.Eilemann@epfl.ch
 */

#define BOOST_TEST_MODULE zeq_publisher

#include "broker.h"

#include <lunchbox/servus.h>

BOOST_AUTO_TEST_CASE(test_create_uri_publisher)
{
    zeq::Publisher publisher( lunchbox::URI( "foo://" ));
}

BOOST_AUTO_TEST_CASE(test_create_invalid_uri_publisher)
{
    // invalid URI, no hostname allowed
    BOOST_CHECK_THROW(
        zeq::Publisher publisher( lunchbox::URI( "foo://localhost" )),
        std::runtime_error );
}

BOOST_AUTO_TEST_CASE(test_publish)
{
    zeq::Publisher publisher( test::buildURI( "*" ));
    BOOST_CHECK( publisher.publish(
                     zeq::vocabulary::serializeEcho( test::echoMessage )));
}

BOOST_AUTO_TEST_CASE(test_publish_empty_event)
{
    zeq::Publisher publisher( test::buildURI( "*" ));
    BOOST_CHECK( publisher.publish( zeq::Event( zeq::vocabulary::EVENT_EXIT )));
}

BOOST_AUTO_TEST_CASE(test_multiple_publisher_on_same_host)
{
    if( !lunchbox::Servus::isAvailable() || getenv("TRAVIS"))
        return;

    const lunchbox::uint128_t uuid = lunchbox::make_UUID();
    const std::string scheme = boost::lexical_cast< std::string >( uuid.high( )) +
                               boost::lexical_cast< std::string >( uuid.low( ));
    const lunchbox::URI uri( scheme + "://*:0" );

    const zeq::Publisher publisher1( uri );
    const zeq::Publisher publisher2( uri );
    const zeq::Publisher publisher3( uri );

    lunchbox::Servus service( std::string( "_" ) + scheme + "._tcp" );
    const lunchbox::Strings& instances =
            service.discover( lunchbox::Servus::IF_ALL, 1000 );
    BOOST_CHECK_EQUAL( instances.size(), 3 );
}
