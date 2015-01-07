
/* Copyright (c) 2014-2015, Human Brain Project
 *                          Stefan.Eilemann@epfl.ch
 */

#include "broker.h"
#include "../detail/socket.h"
#include "../receiver.h"

#include <boost/foreach.hpp>
#include <lunchbox/log.h>
#include <lunchbox/servus.h>
#include <map>

namespace zeq
{
namespace connection
{
namespace detail
{
class Broker
{
public:
    Broker( Receiver& receiver, const std::string& address, void* context )
        : _receiver( receiver )
        , _socket( zmq_socket( context, ZMQ_REP ))
    {
        const std::string zmqAddr( std::string( "tcp://" ) + address );
        if( zmq_bind( _socket, zmqAddr.c_str( )) == -1 )
        {
            zmq_close( _socket );
            LBTHROW( std::runtime_error(
                         "Cannot connect broker to " + zmqAddr + ": " +
                         zmq_strerror( zmq_errno( ))));
        }
        LBINFO << "Bound broker to " << zmqAddr << std::endl;
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
};
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
