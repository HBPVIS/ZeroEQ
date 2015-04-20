
/* Copyright (c) 2014-2015, Human Brain Project
 *                          Stefan.Eilemann@epfl.ch
 */

#include "broker.h"
#include <zeq/detail/port.h>
#include <zeq/detail/socket.h>
#include <zeq/receiver.h>

#include <lunchbox/log.h>
#include <lunchbox/servus.h>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <map>

namespace zeq
{
namespace connection
{
namespace detail
{
using boost::lexical_cast;

class Broker
{
public:
    Broker( const std::string& name, Receiver& receiver,
            const connection::Broker::PortSelection mode, void* context )
        : _receiver( receiver )
        , _socket( zmq_socket( context, ZMQ_REP ))
    {
        const std::string zmqAddr( std::string( "tcp://*:" ) +
                    lexical_cast< std::string >( zeq::detail::getPort( name )));

        _listen( zmqAddr, mode ) ||
            _listen( "tcp://*:0", connection::Broker::PORT_FIXED );
    }

    Broker( Receiver& receiver, const std::string& address, void* context )
        : _receiver( receiver )
        , _socket( zmq_socket( context, ZMQ_REP ))
    {
        _listen( std::string( "tcp://" ) + address,
                 connection::Broker::PORT_FIXED );
    }

    ~Broker()
    {
        if( _socket )
            zmq_close( _socket );
    }

    void addSockets( std::vector< zeq::detail::Socket >& entries )
    {
        zeq::detail::Socket entry;
        entry.socket = _socket;
        entry.events = ZMQ_POLLIN;
        entries.push_back( entry );
    }

    void process( zeq::detail::Socket& socket )
    {
        zmq_msg_t msg;
        zmq_msg_init( &msg );
        zmq_msg_recv( &msg, socket.socket, 0 );
        const std::string address( (const char*)zmq_msg_data( &msg ),
                                   zmq_msg_size( &msg ));

        _receiver.addConnection( std::string( "tcp://" ) + address );
        zmq_msg_send( &msg, socket.socket, 0 );
        zmq_msg_close( &msg );
    }

private:
    zeq::Receiver& _receiver;
    void* _socket;

    bool _listen( const std::string& address,
                  const connection::Broker::PortSelection mode )
    {
        if( zmq_bind( _socket, address.c_str( )) == -1 )
        {
            if( mode == connection::Broker::PORT_FIXED )
            {
                zmq_close( _socket );
                LBTHROW( std::runtime_error(
                             "Cannot connect broker to " + address + ": " +
                              zmq_strerror( zmq_errno( ))));
            }
            return false;
        }

        LBINFO << "Bound broker to " << address << std::endl;
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

}
}
