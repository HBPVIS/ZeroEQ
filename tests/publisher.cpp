
/* Copyright (c) 2015-2016, Human Brain Project
 *                          Daniel Nachbaur <daniel.nachbaur@epfl.ch>
 *                          Stefan.Eilemann@epfl.ch
 *                          Juan Hernando <jhernando@fi.upm.es>
 */

#define BOOST_TEST_MODULE zeq_publisher

#include "broker.h"
#include <zeq/detail/broker.h>
#include <zeq/detail/constants.h>
#include <zeq/detail/sender.h>

#include <servus/servus.h>

#ifdef _MSC_VER
#  define setenv( name, value, overwrite ) \
    _putenv_s( name, value )
#  define unsetenv( name ) \
    _putenv_s( name, nullptr )
#endif

BOOST_AUTO_TEST_CASE(create_uri_publisher)
{
    const zeq::Publisher publisher( zeq::URI( "" ));

    const zeq::URI& uri = publisher.getURI();
    const std::string expectedScheme( "tcp" );
    const std::string baseScheme = uri.getScheme().substr( 0,
                                                      expectedScheme.length( ));
    BOOST_CHECK_EQUAL( baseScheme, expectedScheme );
    BOOST_CHECK( !uri.getHost().empty( ));
    BOOST_CHECK( uri.getPort() > 1024 );
}

BOOST_AUTO_TEST_CASE(create_invalid_uri_publisher)
{
    // invalid URI, hostname only not allowed
    BOOST_CHECK_THROW(
        zeq::Publisher publisher( zeq::URI( "localhost" )),
        std::runtime_error );
}

BOOST_AUTO_TEST_CASE(publish)
{
    zeq::Publisher publisher( zeq::NULL_SESSION );
#ifdef ZEQ_USE_ZEROBUF
    BOOST_CHECK( publisher.publish( zeq::vocabulary::Echo( test::echoMessage)));
#endif
    BOOST_CHECK( publisher.publish(
                     zeq::vocabulary::serializeEcho( test::echoMessage )));
}

BOOST_AUTO_TEST_CASE(publish_update_uri)
{
    zeq::Publisher publisher( zeq::NULL_SESSION );
    const zeq::URI& uri = publisher.getURI();
    BOOST_CHECK_MESSAGE( uri.getPort() != 0, uri );
    BOOST_CHECK_MESSAGE( !uri.getHost().empty(), uri );
#ifdef ZEQ_USE_ZEROBUF
    BOOST_CHECK( publisher.publish( zeq::vocabulary::Echo( test::echoMessage)));
#endif
    BOOST_CHECK( publisher.publish(
                     zeq::vocabulary::serializeEcho( test::echoMessage )));
}

BOOST_AUTO_TEST_CASE(publish_empty_event)
{
    zeq::Publisher publisher( zeq::NULL_SESSION );
    BOOST_CHECK( publisher.publish( zeq::Event( zeq::vocabulary::EVENT_EXIT )));
}

BOOST_AUTO_TEST_CASE(multiple_publisher_on_same_host)
{
    if( !servus::Servus::isAvailable() || getenv("TRAVIS"))
        return;

    const zeq::Publisher publisher1;
    const zeq::Publisher publisher2;
    const zeq::Publisher publisher3;

    servus::Servus service( PUBLISHER_SERVICE );
    const servus::Strings& instances =
            service.discover( servus::Servus::IF_ALL, 1000 );
    BOOST_CHECK_EQUAL( instances.size(), 3 );
}

BOOST_AUTO_TEST_CASE(zeroconf_record)
{
    if( !servus::Servus::isAvailable() || getenv("TRAVIS"))
        return;

    const zeq::Publisher publisher;

    servus::Servus service( PUBLISHER_SERVICE );
    const servus::Strings& instances =
            service.discover( servus::Servus::IF_ALL, 1000 );
    BOOST_REQUIRE_EQUAL( instances.size(), 1 );

    const std::string& instance = instances[0];
    BOOST_CHECK_EQUAL( instance, publisher.getAddress( ));
    BOOST_CHECK_EQUAL( service.get( instance, KEY_APPLICATION ), "publisher" );
    BOOST_CHECK_EQUAL( zeq::uint128_t( service.get( instance, KEY_INSTANCE )),
                       zeq::detail::Sender::getUUID( ));
    BOOST_CHECK_EQUAL( service.get( instance, KEY_SESSION ), getUserName( ));
    BOOST_CHECK_EQUAL( service.get( instance, KEY_USER ), getUserName( ));
}

BOOST_AUTO_TEST_CASE(custom_session)
{
    if( !servus::Servus::isAvailable() || getenv("TRAVIS"))
        return;

    const zeq::Publisher publisher( test::buildUniqueSession( ));

    servus::Servus service( PUBLISHER_SERVICE );
    const servus::Strings& instances =
            service.discover( servus::Servus::IF_ALL, 1000 );
    BOOST_REQUIRE_EQUAL( instances.size(), 1 );

    const std::string& instance = instances[0];
    BOOST_CHECK_EQUAL( service.get( instance, KEY_SESSION ),
                       publisher.getSession( ));
}

BOOST_AUTO_TEST_CASE(different_session_at_runtime)
{
    if( !servus::Servus::isAvailable() || getenv("TRAVIS"))
        return;

    setenv( "ZEROEQ_SESSION", "testsession", 1 );
    const zeq::Publisher publisher;

    servus::Servus service( PUBLISHER_SERVICE );
    const servus::Strings& instances =
            service.discover( servus::Servus::IF_ALL, 1000 );
    BOOST_REQUIRE_EQUAL( instances.size(), 1 );

    const std::string& instance = instances[0];
    BOOST_CHECK_EQUAL( service.get( instance, KEY_SESSION ),
                       "testsession" );
    unsetenv( "ZEROEQ_SESSION" );
}

BOOST_AUTO_TEST_CASE(empty_session)
{
    BOOST_CHECK_THROW( const zeq::Publisher publisher( "" ),
                       std::runtime_error );
}

BOOST_AUTO_TEST_CASE(empty_session_from_environment)
{
    setenv( "ZEROEQ_SESSION", "", 1 );

    const zeq::Publisher publisher;
    BOOST_CHECK_EQUAL( publisher.getSession(), getUserName( ));

    unsetenv( "ZEROEQ_SESSION" );
}

BOOST_AUTO_TEST_CASE(fixed_uri_and_session)
{
    if( !servus::Servus::isAvailable() || getenv("TRAVIS"))
        return;

    const zeq::Publisher publisher( zeq::URI( "127.0.0.1"),
                                    test::buildUniqueSession( ));
    servus::Servus service( PUBLISHER_SERVICE );
    const servus::Strings& instances =
            service.discover( servus::Servus::IF_ALL, 1000 );
    BOOST_REQUIRE_EQUAL( instances.size(), 1 );
}

BOOST_AUTO_TEST_CASE(legacy_ctor)
{
    if( !servus::Servus::isAvailable() || getenv("TRAVIS"))
        return;

    const zeq::Publisher publisher( servus::URI( "foo://" ));

    servus::Servus service( PUBLISHER_SERVICE );
    const servus::Strings& instances =
            service.discover( servus::Servus::IF_ALL, 1000 );
    BOOST_CHECK_EQUAL( instances.size(), 1 );
    BOOST_CHECK_EQUAL( service.get( instances[0], KEY_SESSION ),
                       getUserName( ));
}
