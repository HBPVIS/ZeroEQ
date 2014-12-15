
/* Copyright (c) 2014, Human Brain Project
 *                     Daniel Nachbaur <daniel.nachbaur@epfl.ch>
 *                     Stefan.Eilemann@epfl.ch
 */

#include <zeq/zeq.h>
#include <lunchbox/rng.h>
#include <lunchbox/uri.h>

#include <boost/test/unit_test.hpp>

namespace test
{
lunchbox::URI buildURI( const std::string& hostname = std::string( ))
{
    std::ostringstream uri;
    uri << "foo://" << (hostname.empty() ? "localhost" : hostname)
        << ":" << (lunchbox::RNG().get<uint16_t>() % 60000) + 1024;
    return lunchbox::URI( uri.str( ));
}

const std::string echoMessage( "echo_event" );

void onEchoEvent( const zeq::Event& event )
{
    BOOST_CHECK( event.getType() == zeq::vocabulary::EVENT_ECHO );
    const std::string message = zeq::vocabulary::deserializeEcho( event );
    BOOST_CHECK_EQUAL( echoMessage, message );
}

void onExitEvent( const zeq::Event& event )
{
    BOOST_CHECK_EQUAL( event.getType(), zeq::vocabulary::EVENT_EXIT );
    BOOST_CHECK_EQUAL( event.getSize(), 0 );
}
}
