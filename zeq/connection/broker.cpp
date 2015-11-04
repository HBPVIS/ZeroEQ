
/* Copyright (c) 2014-2015, Human Brain Project
 *                          Stefan.Eilemann@epfl.ch
 */

#include "broker.h"

#include <zeq/detail/port.h>
#include <zeq/detail/sender.h>
#include <zeq/detail/socket.h>
#include <zeq/receiver.h>
#include <zeq/log.h>

#include <map>
#include <cassert>

namespace zeq
{
namespace connection
{
namespace detail
{
class Broker : public zeq::detail::Sender
{
public:
    Broker( const std::string& name, Receiver& receiver,
            const connection::Broker::PortSelection mode, void* context )
        : Sender( URI( std::string( "tcp://*:" ) +
                       std::to_string( uint32_t( zeq::detail::getPort( name)))),
                  context, ZMQ_REP )
        , _receiver( receiver )
    {
        if( !_listen( mode ))
        {
            uri = URI( "tcp://*:0" );
            _listen( connection::Broker::PORT_FIXED );
        }
        initURI();
    }

    Broker( Receiver& receiver, const std::string& address, void* context )
        : Sender( URI( std::string( "tcp://" ) + address ), context, ZMQ_REP )
        , _receiver( receiver )
    {
        _listen( connection::Broker::PORT_FIXED );
        initURI();
    }

    ~Broker() {}

    void addSockets( std::vector< zeq::detail::Socket >& entries )
    {
        assert( socket );
        if( !socket )
            return;

        zeq::detail::Socket entry;
        entry.socket = socket;
        entry.events = ZMQ_POLLIN;
        entries.push_back( entry );
    }

    void process( zeq::detail::Socket& socket_ )
    {
        zmq_msg_t msg;
        zmq_msg_init( &msg );
        zmq_msg_recv( &msg, socket_.socket, 0 );
        const std::string address( (const char*)zmq_msg_data( &msg ),
                                   zmq_msg_size( &msg ));

        _receiver.addConnection( std::string( "tcp://" ) + address );
        zmq_msg_send( &msg, socket_.socket, 0 );
        zmq_msg_close( &msg );
    }

private:
    zeq::Receiver& _receiver;

    bool _listen( const connection::Broker::PortSelection mode )
    {
        const std::string address = std::to_string( uri ) +
                                    ( uri.getPort() ? "" : ":0" );
        if( zmq_bind( socket, address.c_str( )) == -1 )
        {
            if( mode == connection::Broker::PORT_FIXED )
            {
                zmq_close( socket );
                socket = 0;
                ZEQTHROW( std::runtime_error(
                              "Cannot connect broker to " + address + ": " +
                              zmq_strerror( zmq_errno( ))));
            }
            return false;
        }

        ZEQINFO << "Bound broker to " << address << std::endl;
        return true;
    }
};
}

Broker::Broker( const std::string& name, Receiver& receiver,
                const PortSelection mode )
    : Receiver( receiver )
    , _impl( new detail::Broker( name, receiver, mode, getZMQContext( )))
{
}

Broker::Broker( const std::string& address, Receiver& receiver )
    : Receiver( receiver )
    , _impl( new detail::Broker( receiver, address, getZMQContext( )))
{
}

Broker::~Broker()
{
    delete _impl;
}

void Broker::addSockets( std::vector< zeq::detail::Socket >& entries )
{
    _impl->addSockets( entries );
}

void Broker::process( zeq::detail::Socket& socket )
{
    _impl->process( socket );
}

std::string Broker::getAddress() const
{
    return _impl->getAddress();
}

}
}
