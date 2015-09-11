
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

#ifdef _WIN32
#  include <process.h>
#  define getpid _getpid
#else
#  include <sys/types.h>
#  include <unistd.h>
#endif

namespace test
{
zeq::URI buildPublisherURI()
{
    std::string name = std::string(
        boost::unit_test::framework::current_test_case().p_name );
    std::replace( name.begin(), name.end(), '_', '-' );
    return zeq::URI( name + std::to_string( getpid( )) + "://" );
}

zeq::URI buildURI( const std::string& hostname )
{
    std::string name = std::string(
        boost::unit_test::framework::current_test_case().p_name );
    std::replace( name.begin(), name.end(), '_', '-' );
    return zeq::URI( name + std::to_string( getpid( )) + "://" + hostname );
}

zeq::URI buildURI( const std::string& hostname, const zeq::Publisher& to )
{
    assert( to.getPort( ));
    zeq::URI uri = buildURI( hostname );
    uri.setPort( to.getPort( ));
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
