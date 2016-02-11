
/* Copyright (c) 2014-2016, Human Brain Project
 *                          Daniel Nachbaur <daniel.nachbaur@epfl.ch>
 *                          Stefan.Eilemann@epfl.ch
 *                          Juan Hernando <jhernando@fi.upm.es>
 */

#include <zeq/detail/port.h>
#include <zeq/zeq.h>

#include <servus/serializable.h>
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
std::string buildUniqueSession()
{
    std::string name = std::string(
        boost::unit_test::framework::current_test_case().p_name );
    std::replace( name.begin(), name.end(), '_', '-' );
    return name + std::to_string( getpid( ));
}

zeq::URI buildURI( const std::string& hostname, const zeq::Publisher& to )
{
    assert( to.getURI().getPort( ));
    zeq::URI uri;
    uri.setHost( hostname );
    uri.setPort( to.getURI().getPort( ));
    return uri;
}

const std::string echoMessage( "So long, and thanks for all the fish!" );

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

class Echo : public servus::Serializable
{
public:
    std::string getTypeName() const final { return "zeq::test::Echo"; }

    Echo() {}
    Echo( const std::string& message ) : _message( message ) {}
    const std::string& getMessage() const { return _message; }

    bool operator == ( const Echo& rhs ) const { return _message == rhs._message; }

private:
    bool _fromBinary( const void* data, const size_t ) final
    {
        _message = std::string( static_cast< const char* >( data ));
        return true;
    }

    Data _toBinary() const final
    {
        Data data;
        data.ptr = std::shared_ptr< const void >( _message.c_str(),
                                                  []( const void* ){} );
        data.size = _message.length();
        return data;
    }

    std::string _message;
};

}
