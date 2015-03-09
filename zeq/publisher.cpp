
/* Copyright (c) 2014-2015, Human Brain Project
 *                          Daniel Nachbaur <daniel.nachbaur@epfl.ch>
 *                          Stefan.Eilemann@epfl.ch
 */

#include "publisher.h"
#include "event.h"
#include "detail/broker.h"

#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <lunchbox/bitOperation.h>
#include <lunchbox/log.h>
#include <lunchbox/servus.h>
#include <zmq.h>
#include <map>

// for NI_MAXHOST
#ifdef _WIN32
#  include <Ws2tcpip.h>
#else
#  include <netdb.h>
#endif

using boost::lexical_cast;

namespace zeq
{
namespace detail
{

class Publisher
{
public:
    Publisher( const lunchbox::URI& uri )
        : _context( zmq_ctx_new( ))
        , _publisher( zmq_socket( _context, ZMQ_PUB ))
        , _service( std::string( "_" ) + uri.getScheme() + "._tcp" )
    {
        const std::string& zmqURI = buildZmqURI( uri );
        if( zmq_bind( _publisher, zmqURI.c_str( )) == -1 )
        {
            zmq_close( _publisher );
            zmq_ctx_destroy( _context );
            _publisher = 0;

            LBTHROW( std::runtime_error(
                         std::string( "Cannot bind publisher socket '" ) +
                         zmqURI + "': " + zmq_strerror( zmq_errno( ))));
        }

        _initService( uri.getHost(), uri.getPort( ));
    }

    ~Publisher()
    {
        zmq_close( _publisher );
        zmq_ctx_destroy( _context );
    }

    bool publish( const zeq::Event& event )
    {
#ifdef LB_LITTLEENDIAN
        const uint128_t& type = event.getType();
#else
        uint128_t type = event.getType();
        lunchbox::byteswap( type ); // convert to little endian wire protocol
#endif
        zmq_msg_t msgHeader;
        zmq_msg_init_size( &msgHeader, sizeof( type ));
        memcpy( zmq_msg_data( &msgHeader ), &type, sizeof( type ));
        int ret = zmq_msg_send( &msgHeader, _publisher,
                                event.getSize() > 0 ? ZMQ_SNDMORE : 0 );
        zmq_msg_close( &msgHeader );
        if( ret == -1 )
        {
            LBWARN << "Cannot publish message header, got "
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
            LBWARN << "Cannot publish message data, got "
                   << zmq_strerror( zmq_errno( )) << std::endl;
            return false;
        }
        return true;
    }

    const std::string& getAddress() const { return _address; }

private:
    void _initService( std::string host, uint16_t port )
    {
        _initAddress( host, port );
        if( lunchbox::Servus::isAvailable( ))
            _service.announce( port, _address );
    }

    void _initAddress( std::string& host, uint16_t& port )
    {
        if( host == "*" )
            host.clear();

        if( host.empty() || port == 0 )
        {
            char endPoint[1024];
            size_t size = sizeof( endPoint );
            if( zmq_getsockopt( _publisher, ZMQ_LAST_ENDPOINT, &endPoint,
                                &size ) == -1 )
            {
                LBTHROW( std::runtime_error(
                             "Cannot determine port of publisher" ));
            }

            const std::string endPointStr( endPoint );

            if( port == 0 )
            {
                const std::string portStr =
                    endPointStr.substr( endPointStr.find_last_of( ":" ) + 1 );
                port = lexical_cast< uint16_t >( portStr );
            }

            if( host.empty( ))
            {
                const size_t start = endPointStr.find_last_of( "/" ) + 1;
                const size_t end = endPointStr.find_last_of( ":" );
                host = endPointStr.substr( start, end - start );
                if( host == "0.0.0.0" )
                {
                    char hostname[NI_MAXHOST+1] = {0};
                    gethostname( hostname, NI_MAXHOST );
                    hostname[NI_MAXHOST] = '\0';
                    host = hostname;
                }
            }

            LBINFO << "Bound " << _service.getName() << " publisher to " << host
                   << ":" << port << std::endl;
        }

        _address = host + ":" + lexical_cast< std::string >( port );
    }

    void* _context;
    void* _publisher;
    lunchbox::Servus _service;
    std::string _address;
};
}

Publisher::Publisher( const lunchbox::URI& uri )
    : _impl( new detail::Publisher( uri ))
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

const std::string& Publisher::getAddress() const
{
    return _impl->getAddress();
}

}
