
/* Copyright (c) 2014-2015, Human Brain Project
 *                          Daniel Nachbaur <daniel.nachbaur@epfl.ch>
 *                          Stefan.Eilemann@epfl.ch
 */

#include "publisher.h"
#include "event.h"
#include "log.h"
#include "detail/broker.h"
#include "detail/byteswap.h"
#include "detail/constants.h"
#include "detail/sender.h"

#include <servus/servus.h>
#ifdef ZEQ_USE_ZEROBUF
#  include <zerobuf/Zerobuf.h>
#endif

#include <cstring>
#include <map>

namespace zeq
{

namespace
{
std::string _getApplicationName()
{
    // http://stackoverflow.com/questions/933850
#ifdef _MSC_VER
    char result[MAX_PATH];
    const std::string execPath( result, GetModuleFileName( NULL, result,
                                                           MAX_PATH ));
#elif __APPLE__
    char result[PATH_MAX+1];
    uint32_t size = sizeof(result);
    if( _NSGetExecutablePath( result, &size ) != 0 )
        return std::string();
    const std::string execPath( result );
#else
    char result[PATH_MAX];
    const ssize_t count = readlink( "/proc/self/exe", result, PATH_MAX );
    if( count < 0 )
    {
        // Not all UNIX have /proc/self/exe
        ZEQWARN << "Could not find absolute executable path" << std::endl;
        return std::string();
    }
    const std::string execPath( result, count > 0 ? count : 0 );
#endif

#ifdef _MSC_VER
    const size_t lastSeparator = execPath.find_last_of('\\');
#else
    const size_t lastSeparator = execPath.find_last_of('/');
#endif
    if( lastSeparator == std::string::npos )
        return execPath;
    // lastSeparator + 1 may be at most equal to filename.size(), which is good
    return execPath.substr( lastSeparator + 1 );
}
}

class Publisher::Impl : public detail::Sender
{
public:
    Impl( servus::URI uri_, const uint32_t announceMode )
        : detail::Sender( uri_, 0, ZMQ_PUB )
        , _service( PUBLISHER_SERVICE )
        , _session( getDefaultSession( ))
    {
        uri_.setScheme( "" );
        const std::string& zmqURI = buildZmqURI( uri_ );
        if( zmq_bind( socket, zmqURI.c_str( )) == -1 )
        {
            zmq_close( socket );
            socket = 0;
            ZEQTHROW( std::runtime_error(
                          std::string( "Cannot bind publisher socket '" ) +
                          zmqURI + "': " + zmq_strerror( zmq_errno( ))));
        }

        initURI();
        _initService( announceMode );
    }

    Impl( const URI& uri_, const std::string& session )
        : detail::Sender( uri_, 0, ZMQ_PUB )
        , _service( PUBLISHER_SERVICE )
        , _session( session == DEFAULT_SESSION ? getDefaultSession() : session )
    {
        if( session.empty( ))
            ZEQTHROW( std::runtime_error(
                          "Empty session is not allowed for publisher" ));

        const std::string& zmqURI = buildZmqURI( uri );
        if( zmq_bind( socket, zmqURI.c_str( )) == -1 )
        {
            zmq_close( socket );
            socket = 0;
            ZEQTHROW( std::runtime_error(
                          std::string( "Cannot bind publisher socket '" ) +
                          zmqURI + "': " + zmq_strerror( zmq_errno( ))));
        }

        initURI();

        if( session != NULL_SESSION )
            _initService();
    }

    ~Impl() {}

    bool publish( const zeq::Event& event )
    {
#ifdef COMMON_LITTLEENDIAN
        const uint128_t& type = event.getType();
#else
        uint128_t type = event.getType();
        detail::byteswap( type ); // convert to little endian wire protocol
#endif
        zmq_msg_t msgHeader;
        zmq_msg_init_size( &msgHeader, sizeof( type ));
        memcpy( zmq_msg_data( &msgHeader ), &type, sizeof( type ));
        int ret = zmq_msg_send( &msgHeader, socket,
                                event.getSize() > 0 ? ZMQ_SNDMORE : 0 );
        zmq_msg_close( &msgHeader );
        if( ret == -1 )
        {
            ZEQWARN << "Cannot publish message header, got "
                   << zmq_strerror( zmq_errno( )) << std::endl;
            return false;
        }

        if( event.getSize() == 0 )
            return true;

        zmq_msg_t msg;
        zmq_msg_init_size( &msg, event.getSize( ));
        memcpy( zmq_msg_data(&msg), event.getData(), event.getSize( ));
        ret = zmq_msg_send( &msg, socket, 0 );
        zmq_msg_close( &msg );
        if( ret  == -1 )
        {
            ZEQWARN << "Cannot publish message data, got "
                    << zmq_strerror( zmq_errno( )) << std::endl;
            return false;
        }
        return true;
    }

#ifdef ZEQ_USE_ZEROBUF
    bool publish( const zerobuf::Zerobuf& zerobuf )
    {
        // TODO: Save type in zerobuf and transmit in one message
#ifdef COMMON_LITTLEENDIAN
        const uint128_t& type = zerobuf.getZerobufType();
#else
        uint128_t type = zerobuf.getZerobufType();
        detail::byteswap( type ); // convert to little endian wire protocol
#endif
        const void* data = zerobuf.getZerobufData();

        zmq_msg_t msgHeader;
        zmq_msg_init_size( &msgHeader, sizeof( type ));
        memcpy( zmq_msg_data( &msgHeader ), &type, sizeof( type ));
        int ret = zmq_msg_send( &msgHeader, socket,
                                data ? ZMQ_SNDMORE : 0 );
        zmq_msg_close( &msgHeader );
        if( ret == -1 )
        {
            ZEQWARN << "Cannot publish message header, got "
                   << zmq_strerror( zmq_errno( )) << std::endl;
            return false;
        }

        if( !data )
            return true;

        zmq_msg_t msg;
        zmq_msg_init_size( &msg, zerobuf.getZerobufSize( ));
        ::memcpy( zmq_msg_data(&msg), data, zerobuf.getZerobufSize( ));
        ret = zmq_msg_send( &msg, socket, 0 );
        zmq_msg_close( &msg );
        if( ret  == -1 )
        {
            ZEQWARN << "Cannot publish message data, got "
                    << zmq_strerror( zmq_errno( )) << std::endl;
            return false;
        }
        return true;
    }
#endif

    const std::string& getSession() const { return _session; }

private:
    void _initService( const uint32_t announceMode = ANNOUNCE_REQUIRED )
    {
        if( !( announceMode & (ANNOUNCE_ZEROCONF | ANNOUNCE_REQUIRED) ))
            return;

        const bool required = announceMode & ANNOUNCE_REQUIRED;
        if( !servus::Servus::isAvailable( ))
        {
            if( required )
                ZEQTHROW( std::runtime_error(
                              "No zeroconf implementation available" ));
            return;
        }

        _service.set( KEY_INSTANCE, detail::Sender::getUUID().getString( ));
        _service.set( KEY_USER, getUserName( ));
        _service.set( KEY_APPLICATION, _getApplicationName( ));
        if( !_session.empty( ))
            _service.set( KEY_SESSION, _session );

        const servus::Servus::Result& result =
            _service.announce( uri.getPort(), getAddress( ));

        if( required && !result )
        {
            ZEQTHROW( std::runtime_error( "Zeroconf announce failed: " +
                                          result.getString( )));
        }
    }

    servus::Servus _service;
    const std::string _session;
};

Publisher::Publisher()
    : _impl( new Impl( URI(), DEFAULT_SESSION ))
{
}

Publisher::Publisher( const std::string& session )
    : _impl( new Impl( URI(), session ))
{}

Publisher::Publisher( const URI& uri )
    : _impl( new Impl( uri, DEFAULT_SESSION ))
{}

Publisher::Publisher( const URI& uri, const std::string& session )
    : _impl( new Impl( uri, session ))
{}

Publisher::Publisher( const servus::URI& uri, const uint32_t announceMode )
    : _impl( new Impl( uri, announceMode ))
{
    ZEQWARN << "zeq::Publisher( const servus::URI&, uint32_t ) is deprecated"
            << std::endl;
}

Publisher::~Publisher()
{
}

bool Publisher::publish( const Event& event )
{
    return _impl->publish( event );
}

#ifdef ZEQ_USE_ZEROBUF
bool Publisher::publish( const zerobuf::Zerobuf& zerobuf )
{
    return _impl->publish( zerobuf );
}
#else
bool Publisher::publish( const zerobuf::Zerobuf& )
{
    ZEQTHROW( std::runtime_error( "ZeroEQ not built with ZeroBuf support "));
}
#endif

std::string Publisher::getAddress() const
{
    return _impl->getAddress();
}

const std::string& Publisher::getSession() const
{
    return _impl->getSession();
}

const servus::URI& Publisher::getURI() const
{
    return _impl->uri.toServusURI();
}

}
