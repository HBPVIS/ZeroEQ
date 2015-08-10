
/* Copyright (c) 2014-2015, Human Brain Project
 *                          Daniel Nachbaur <daniel.nachbaur@epfl.ch>
 *                          Stefan.Eilemann@epfl.ch
 *                          Juan Hernando <jhernando@fi.upm.es>
 */

#include <zeq/detail/port.h>
#include <zeq/zeq.h>

#ifdef ZEQ_USE_ZEROBUF
#  include <echo.h>
#endif

#include <servus/uri.h>
#include <boost/test/unit_test.hpp>
#include <string>

namespace test
{
zeq::URI buildPublisherURI()
{
    std::string name = std::string(
        boost::unit_test::framework::current_test_case().p_name );
    std::replace( name.begin(), name.end(), '_', '-' );
    return zeq::URI( name + "://" );
}

zeq::URI buildPublisherURI( const unsigned int port )
{
    zeq::URI uri = buildPublisherURI();
    uri.setPort( port );
    return uri;
}

zeq::URI buildURI( const std::string& hostname )
{
    std::string name = std::string(
        boost::unit_test::framework::current_test_case().p_name );
    std::replace( name.begin(), name.end(), '_', '-' );
    return zeq::URI( name + "://" + hostname );
}

zeq::URI buildURI( const std::string& hostname, const unsigned int port )
{
    zeq::URI uri = buildURI( hostname );
    uri.setPort( port );
    return uri;
}

const std::string echoMessage( "echo_event" );

void onEchoEvent( const zeq::Event& event )
{
    BOOST_CHECK( event.getType() == zeq::vocabulary::EVENT_ECHO );
    const std::string message = zeq::vocabulary::deserializeEcho( event );
    BOOST_CHECK_EQUAL( echoMessage, message );
}

#ifdef ZEQ_USE_ZEROBUF
class EchoOut : public zeq::vocabulary::Echo
{
public:
    EchoOut() { setMessage( "So long, and thanks for all the fish!" ); }
};

class EchoIn : public zeq::vocabulary::Echo
{
    void notifyUpdated() final
    {
        BOOST_CHECK_EQUAL( getMessageString(),
                           "So long, and thanks for all the fish!" );
        gotData = true;
    }

public:
    bool gotData;

    EchoIn() : gotData( false ) {}
};
#endif

void onExitEvent( const zeq::Event& event )
{
    BOOST_CHECK_EQUAL( event.getType(), zeq::vocabulary::EVENT_EXIT );
    BOOST_CHECK_EQUAL( event.getSize(), 0 );
}

}
