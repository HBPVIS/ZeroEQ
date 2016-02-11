
/* Copyright (c) 2014, Human Brain Project
 *                     Daniel Nachbaur <daniel.nachbaur@epfl.ch>
 *                     Stefan.Eilemann@epfl.ch
 */

#define BOOST_TEST_MODULE zeq_subscriber

#include "broker.h"

#include <servus/servus.h>

using namespace zeq::vocabulary;

BOOST_AUTO_TEST_CASE(construction)
{
    BOOST_CHECK_NO_THROW( zeq::Subscriber( ));
    BOOST_CHECK_NO_THROW(
                zeq::Subscriber subscriber( test::buildUniqueSession( )));
    BOOST_CHECK_NO_THROW( zeq::Subscriber( zeq::URI( "localhost:1234" )));
    BOOST_CHECK_NO_THROW( zeq::Subscriber( zeq::URI( "localhost" ),
                                           zeq::DEFAULT_SESSION ));

    zeq::Subscriber shared;
    BOOST_CHECK_NO_THROW( zeq::Subscriber( (zeq::Receiver&)shared ));
    BOOST_CHECK_NO_THROW(
                zeq::Subscriber( test::buildUniqueSession(), shared ));
    BOOST_CHECK_NO_THROW( zeq::Subscriber( zeq::URI( "localhost:1234" ),
                                           shared ));
    BOOST_CHECK_NO_THROW( zeq::Subscriber( zeq::URI( "localhost" ),
                                           zeq::DEFAULT_SESSION, shared ));
    BOOST_CHECK_NO_THROW( zeq::Subscriber( zeq::URI( "localhost:1234" ),
                                           zeq::DEFAULT_SESSION, shared ));
}

BOOST_AUTO_TEST_CASE(invalid_construction)
{
    BOOST_CHECK_THROW( zeq::Subscriber subscriber( zeq::NULL_SESSION ),
                       std::runtime_error );
    BOOST_CHECK_THROW( zeq::Subscriber( "" ),
                       std::runtime_error );
    BOOST_CHECK_THROW( zeq::Subscriber( zeq::URI( "localhost" )),
                       std::runtime_error );
    BOOST_CHECK_THROW( zeq::Subscriber( zeq::URI( "localhost" ),
                                      zeq::NULL_SESSION ), std::runtime_error );
    BOOST_CHECK_THROW( zeq::Subscriber( zeq::URI( "localhost" ), "" ),
                       std::runtime_error );

    zeq::Subscriber shared;
    BOOST_CHECK_THROW( zeq::Subscriber subscriber( zeq::NULL_SESSION, shared ),
                       std::runtime_error );
    BOOST_CHECK_THROW( zeq::Subscriber( "", shared ),
                       std::runtime_error );
    BOOST_CHECK_THROW( zeq::Subscriber( zeq::URI( "localhost" ), shared),
                       std::runtime_error );
    BOOST_CHECK_THROW( zeq::Subscriber( zeq::URI( "localhost" ),
                                        zeq::NULL_SESSION, shared ),
                       std::runtime_error );
    BOOST_CHECK_THROW( zeq::Subscriber( zeq::URI( "localhost" ), "", shared ),
                       std::runtime_error );
}

BOOST_AUTO_TEST_CASE(registerhandler)
{
    zeq::Subscriber subscriber;
    BOOST_CHECK( !subscriber.hasHandler( EVENT_ECHO ));
    BOOST_CHECK( subscriber.registerHandler( EVENT_ECHO,
                       std::bind( &test::onEchoEvent, std::placeholders::_1 )));
    BOOST_CHECK( subscriber.hasHandler( EVENT_ECHO ));
}

BOOST_AUTO_TEST_CASE(deregisterhandler)
{
    zeq::Subscriber subscriber;
    BOOST_CHECK( subscriber.registerHandler( EVENT_ECHO,
                       std::bind( &test::onEchoEvent, std::placeholders::_1 )));
    BOOST_CHECK( subscriber.deregisterHandler( EVENT_ECHO ));
}

BOOST_AUTO_TEST_CASE(invalid_registerhandler)
{
    zeq::Subscriber subscriber;
    BOOST_CHECK( subscriber.registerHandler( EVENT_ECHO,
                       std::bind( &test::onEchoEvent, std::placeholders::_1 )));
    BOOST_CHECK( !subscriber.registerHandler( EVENT_ECHO,
                       std::bind( &test::onEchoEvent, std::placeholders::_1 )));
}

BOOST_AUTO_TEST_CASE(test_invalid_deregisterhandler)
{
    zeq::Subscriber subscriber;
    BOOST_CHECK( !subscriber.deregisterHandler( EVENT_ECHO ));
    BOOST_CHECK( subscriber.registerHandler( EVENT_ECHO,
                       std::bind( &test::onEchoEvent, std::placeholders::_1 )));
    BOOST_CHECK( !subscriber.deregisterHandler(zeq::vocabulary::EVENT_EXIT ));
}

BOOST_AUTO_TEST_CASE(invalid_subscribe)
{
    zeq::Subscriber subscriber;
    test::Echo echo;
    BOOST_CHECK( subscriber.subscribe( echo ));
    BOOST_CHECK( !subscriber.subscribe( echo ));
}

BOOST_AUTO_TEST_CASE(test_invalid_unsubscribe)
{
    zeq::Subscriber subscriber;
    test::Echo echo;
    BOOST_CHECK( !subscriber.unsubscribe( echo ));
    BOOST_CHECK( subscriber.subscribe( echo ));
    BOOST_CHECK( subscriber.unsubscribe( echo ));
    BOOST_CHECK( !subscriber.unsubscribe( echo ));
}

BOOST_AUTO_TEST_CASE(not_implemented_servus )
{
    if( servus::Servus::isAvailable( ) )
        return;

    const zeq::URI uri( test::buildUniqueSession( ));
    BOOST_CHECK_THROW( zeq::Subscriber subscriber( uri ), std::runtime_error );
}
