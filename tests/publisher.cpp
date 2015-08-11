
/* Copyright (c) 2015, Human Brain Project
 *                     Daniel Nachbaur <daniel.nachbaur@epfl.ch>
 *                     Stefan.Eilemann@epfl.ch
 *                     Juan Hernando <jhernando@fi.upm.es>
 */

#define BOOST_TEST_MODULE zeq_publisher

#include "broker.h"

#include <servus/servus.h>

BOOST_AUTO_TEST_CASE(create_uri_publisher)
{
    zeq::Publisher publisher( test::buildPublisherURI( ));

    const servus::URI& uri = publisher.getURI();
    BOOST_CHECK_EQUAL( uri.getScheme(), std::string( "create-uri-publisher" ));
    BOOST_CHECK( !uri.getHost().empty( ));
    BOOST_CHECK( uri.getPort() > 1024 );
}

BOOST_AUTO_TEST_CASE(create_invalid_uri_publisher)
{
    // invalid URI, hostname only not allowed
    BOOST_CHECK_THROW(
        zeq::Publisher publisher( test::buildURI( "localhost" )),
        std::runtime_error );
}

BOOST_AUTO_TEST_CASE(publish)
{
    zeq::Publisher publisher( test::buildPublisherURI( ));
#ifdef ZEQ_USE_ZEROBUF
    BOOST_CHECK( publisher.publish( test::EchoOut( )));
#endif
    BOOST_CHECK( publisher.publish(
                     zeq::vocabulary::serializeEcho( test::echoMessage )));
}

BOOST_AUTO_TEST_CASE(publish_empty_event)
{
    zeq::Publisher publisher( test::buildPublisherURI( ));
    BOOST_CHECK( publisher.publish( zeq::Event( zeq::vocabulary::EVENT_EXIT )));
}

BOOST_AUTO_TEST_CASE(multiple_publisher_on_same_host)
{
    if( !servus::Servus::isAvailable() || getenv("TRAVIS"))
        return;

    const zeq::uint128_t uuid = servus::make_UUID();
    const std::string scheme = "schema-" + std::to_string( uuid.high( )) +
                               std::to_string( uuid.low( ));
    const servus::URI uri( scheme + "://*:0" );

    const zeq::Publisher publisher1( uri );
    const zeq::Publisher publisher2( uri );
    const zeq::Publisher publisher3( uri );

    servus::Servus service( std::string( "_" ) + scheme + "._tcp" );
    const servus::Strings& instances =
            service.discover( servus::Servus::IF_ALL, 1000 );
    BOOST_CHECK_EQUAL( instances.size(), 3 );
}
