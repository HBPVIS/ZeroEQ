
/* Copyright (c) 2014, Human Brain Project
 *                     Daniel Nachbaur <daniel.nachbaur@epfl.ch>
 *                     Stefan.Eilemann@epfl.ch
 */

#define BOOST_TEST_MODULE zeq_subscriber

#include "broker.h"
#include <lunchbox/servus.h>
#include <boost/bind.hpp>

using namespace zeq::vocabulary;

BOOST_AUTO_TEST_CASE(test_subscribe)
{
    zeq::Subscriber subscriber( test::buildURI( ));
}

BOOST_AUTO_TEST_CASE(test_invalid_subscribe)
{
    BOOST_CHECK_THROW(
        zeq::Subscriber subscriber2( lunchbox::URI( "uri" )),
        std::runtime_error );
}

BOOST_AUTO_TEST_CASE(test_registerhandler)
{
    zeq::Subscriber subscriber( test::buildURI( ));
    BOOST_CHECK( subscriber.registerHandler(
                     EVENT_ECHO, boost::bind( &test::onEchoEvent, _1 )));
}

BOOST_AUTO_TEST_CASE(test_deregisterhandler)
{
    zeq::Subscriber subscriber( test::buildURI( ));
    BOOST_CHECK( subscriber.registerHandler(
                     EVENT_ECHO, boost::bind( &test::onEchoEvent, _1 )));
    BOOST_CHECK( subscriber.deregisterHandler( EVENT_ECHO ));
}

BOOST_AUTO_TEST_CASE(test_invalid_registerhandler)
{
    zeq::Subscriber subscriber( test::buildURI( ));
    BOOST_CHECK( subscriber.registerHandler(
                     EVENT_ECHO, boost::bind( &test::onEchoEvent, _1 )));
    BOOST_CHECK( !subscriber.registerHandler(
                     EVENT_ECHO, boost::bind( &test::onEchoEvent, _1 )));
}

BOOST_AUTO_TEST_CASE(test_invalid_deregisterhandler)
{
    zeq::Subscriber subscriber( test::buildURI( ));
    BOOST_CHECK( !subscriber.deregisterHandler( EVENT_ECHO ));
    BOOST_CHECK( subscriber.registerHandler(
                     EVENT_ECHO, boost::bind( &test::onEchoEvent, _1 )));
    BOOST_CHECK( !subscriber.deregisterHandler(zeq::vocabulary::EVENT_EXIT ));
}

BOOST_AUTO_TEST_CASE(test_not_implemented_servus )
{
    if( lunchbox::Servus::isAvailable( ) )
        return;

    const lunchbox::URI uri( "foo://" );
    BOOST_CHECK_THROW( zeq::Subscriber subscriber( uri ), std::runtime_error );
}
