
/* Copyright (c) 2014-2015, Human Brain Project
 *                          Daniel Nachbaur <daniel.nachbaur@epfl.ch>
 *                          Stefan.Eilemann@epfl.ch
 */

#include "publisher.h"
#include "event.h"
#include "log.h"
#include "detail/broker.h"
#include "detail/byteswap.h"

#include <servus/servus.h>

#include <zmq.h>
#include <map>
#include <string.h>

// for NI_MAXHOST
#ifdef _WIN32
#  include <Ws2tcpip.h>
#else
#  include <unistd.h>
#  include <netdb.h>
#endif

namespace zeq
{
namespace detail
{

class Publisher
{
public:
    Publisher( const servus::URI& uri_, const uint32_t announceMode )
        : uri( uri_ )
        , _context( zmq_ctx_new( ))
        , _publisher( zmq_socket( _context, ZMQ_PUB ))
        , _service( std::string( "_" ) + uri.getScheme() + "._tcp" )
    {
        const std::string& zmqURI = buildZmqURI( uri );
        if( zmq_bind( _publisher, zmqURI.c_str( )) == -1 )
        {
            zmq_close( _publisher );
            zmq_ctx_destroy( _context );
            _publisher = 0;

            ZEQTHROW( std::runtime_error(
                          std::string( "Cannot bind publisher socket '" ) +
                          zmqURI + "': " + zmq_strerror( zmq_errno( ))));
        }

        _initService( announceMode );
    }

    ~Publisher()
    {
        zmq_close( _publisher );
        zmq_ctx_destroy( _context );
    }

    bool publish( const zeq::Event& event )
    {
#ifdef ZEQ_LITTLEENDIAN
        const uint128_t& type = event.getType();
#else
        uint128_t type = event.getType();
        detail::byteswap( type ); // convert to little endian wire protocol
#endif
        zmq_msg_t msgHeader;
        zmq_msg_init_size( &msgHeader, sizeof( type ));
        memcpy( zmq_msg_data( &msgHeader ), &type, sizeof( type ));
        int ret = zmq_msg_send( &msgHeader, _publisher,
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
        ret = zmq_msg_send( &msg, _publisher, 0 );
        zmq_msg_close( &msg );
        if( ret  == -1 )
        {
            ZEQWARN << "Cannot publish message data, got "
                    << zmq_strerror( zmq_errno( )) << std::endl;
            return false;
        }
        return true;
    }

    std::string getAddress() const
    {
        return uri.getHost() + ":" + std::to_string( uint32_t( uri.getPort( )));
    }

    servus::URI uri;

private:
    void _initService( const uint32_t announceMode )
    {
        _initAddress();
        if( !( announceMode & ANNOUNCE_ZEROCONF ))
            return;

        const bool required = announceMode & ANNOUNCE_REQUIRED;
        if( !servus::Servus::isAvailable( ))
        {
            if( required )
                ZEQTHROW( std::runtime_error(
                              "No zeroconf implementation available" ));
            return;
        }

        const servus::Servus::Result& result =
            _service.announce( uri.getPort(), getAddress( ));

        if( required && !result )
        {
            ZEQTHROW( std::runtime_error( "Zeroconf announce failed: " +
                                          result.getString( )));
        }
    }

    void _getEndPoint( std::string& host, std::string& port ) const
    {
        char buffer[1024];
        size_t size = sizeof( buffer );
        if( zmq_getsockopt(
                _publisher, ZMQ_LAST_ENDPOINT, &buffer, &size ) == -1 )
        {
            ZEQTHROW( std::runtime_error(
                          "Cannot determine port of publisher" ));
        }
        const std::string endPoint( buffer );

        port = endPoint.substr( endPoint.find_last_of( ":" ) + 1 );
        const size_t start = endPoint.find_last_of( "/" ) + 1;
        const size_t end = endPoint.find_last_of( ":" );
        host = endPoint.substr( start, end - start );
        if( host == "0.0.0.0" )
        {
            char hostname[NI_MAXHOST+1] = {0};
            gethostname( hostname, NI_MAXHOST );
            hostname[NI_MAXHOST] = '\0';
            host = hostname;
        }
    }

    void _initAddress()
    {
        std::string host = uri.getHost();
        if( host == "*" )
            host.clear();

        uint16_t port = uri.getPort();
        if( host.empty() || port == 0 )
        {
            std::string hostStr, portStr;
            _getEndPoint( hostStr, portStr );

            if( port == 0 )
            {
                // No overflow is possible unless ZMQ reports bad port number.
                port = std::stoi( portStr );
                uri.setPort( port );
            }

            if( host.empty( ))
                uri.setHost( hostStr );

            ZEQINFO << "Bound " << _service.getName() << " publisher to "
                    << host << ":" << port << std::endl;
        }
    }

    void* _context;
    void* _publisher;
    servus::Servus _service;
};
}

Publisher::Publisher( const servus::URI& uri, const uint32_t announceMode )
    : _impl( new detail::Publisher( uri, announceMode ))
{
}

Publisher::~Publisher()
{
    delete _impl;
}

bool Publisher::publish( const Event& event )
{
    return _impl->publish( event );
}

std::string Publisher::getAddress() const
{
    return _impl->getAddress();
}

const servus::URI& Publisher::getURI() const
{
    return _impl->uri;
}

}
