
/* Copyright (c) 2014-2015, Human Brain Project
 *                          Daniel Nachbaur <daniel.nachbaur@epfl.ch>
 *                          Stefan.Eilemann@epfl.ch
 */

#include "publisher.h"
#include "event.h"
#include "log.h"
#include "detail/broker.h"
#include "detail/byteswap.h"
#include "detail/sender.h"

#include <servus/servus.h>
#ifdef ZEQ_USE_ZEROBUF
#  include <zerobuf/Zerobuf.h>
#endif

#include <map>
#include <string.h>

namespace zeq
{
namespace detail
{
class Publisher : public Sender
{
public:
    Publisher( const servus::URI& uri_, const uint32_t announceMode )
        : Sender( uri_, 0, ZMQ_PUB )
        , _service( std::string( "_" ) + uri.getScheme() + "._tcp" )
    {
        const std::string& zmqURI = buildZmqURI( uri );
        if( zmq_bind( socket, zmqURI.c_str( )) == -1 )
        {
            zmq_close( socket );
            socket = 0;
            ZEQTHROW( std::runtime_error(
                          std::string( "Cannot bind publisher socket '" ) +
                          zmqURI + "': " + zmq_strerror( zmq_errno( ))));
        }

        _initService( announceMode );
    }

    ~Publisher() {}

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

private:
    void _initService( const uint32_t announceMode )
    {
        initURI();
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

        _service.set( "Instance", detail::Sender::getUUID().getString( ));

        const servus::Servus::Result& result =
            _service.announce( uri.getPort(), getAddress( ));

        if( required && !result )
        {
            ZEQTHROW( std::runtime_error( "Zeroconf announce failed: " +
                                          result.getString( )));
        }
    }

    servus::Servus _service;
};
}

Publisher::Publisher( const servus::URI& uri, const uint32_t announceMode )
    : _impl( new detail::Publisher( uri, announceMode ))
{
}

Publisher::Publisher( servus::URI& uri, uint32_t announceMode )
    : _impl( new detail::Publisher( uri, announceMode ))
{
    uri = _impl->uri;
}

Publisher::~Publisher()
{
    delete _impl;
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

uint16_t Publisher::getPort() const
{
    return _impl->getPort();
}

const servus::URI& Publisher::getURI() const
{
    return _impl->uri;
}

}
