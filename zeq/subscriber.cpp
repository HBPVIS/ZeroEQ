
/* Copyright (c) 2014-2015, Human Brain Project
 *                          Daniel Nachbaur <daniel.nachbaur@epfl.ch>
 *                          Stefan.Eilemann@epfl.ch
 */

#include "subscriber.h"

#include "event.h"
#include "log.h"
#include "detail/broker.h"
#include "detail/socket.h"
#include "detail/byteswap.h"

#include <servus/servus.h>

#include <cassert>
#include <map>
#include <string.h>

namespace zeq
{
namespace detail
{
class Subscriber
{
public:
    Subscriber( const servus::URI& uri, void* context )
        : _service( std::string( "_" ) + uri.getScheme() + "._tcp" )
    {
        if( uri.getScheme().empty( ))
            ZEQTHROW( std::runtime_error(
                          std::to_string( uri ) +
                          " is not a valid URI (scheme is missing)."));

        if( uri.getHost().empty() || uri.getPort() == 0 )
        {
            if( !servus::Servus::isAvailable( ))
                ZEQTHROW( std::runtime_error(
                              std::string( "Empty servus implementation" )));

            _service.beginBrowsing( servus::Servus::IF_ALL );
            update( context );
        }
        else
        {
            const std::string& zmqURI = buildZmqURI( uri );
            if( !addConnection( context, zmqURI ))
            {
                ZEQTHROW( std::runtime_error(
                              "Cannot connect subscriber to " + zmqURI + ": " +
                               zmq_strerror( zmq_errno( ))));
            }
        }
    }

    ~Subscriber()
    {
        for( const auto& socket : _subscribers )
        {
            if( socket.second )
                zmq_close( socket.second );
        }
        if( _service.isBrowsing( ))
            _service.endBrowsing();
    }

    bool registerHandler( const uint128_t& event, const EventFunc& func )
    {
        if( _eventFuncs.count( event ) != 0 )
            return false;

        // Add subscription to existing sockets
        for( const auto& socket : _subscribers )
        {
            if( socket.second &&
                zmq_setsockopt( socket.second, ZMQ_SUBSCRIBE,
                                &event, sizeof( event )) == -1 )
            {
                throw std::runtime_error(
                    std::string( "Cannot update topic filter: " ) +
                    zmq_strerror( zmq_errno( )));
            }
        }

        _eventFuncs[event] = func;
        return true;
    }

    bool deregisterHandler( const uint128_t& event )
    {
        if( _eventFuncs.erase( event ) == 0 )
            return false;

        for( const auto& socket : _subscribers )
        {
            if( socket.second &&
                zmq_setsockopt( socket.second, ZMQ_UNSUBSCRIBE,
                                &event, sizeof( event )) == -1 )
            {
                throw std::runtime_error(
                    std::string( "Cannot update topic filter: " ) +
                    zmq_strerror( zmq_errno( )));
            }
        }

        return true;
    }

    void addSockets( std::vector< detail::Socket >& entries )
    {
        entries.insert( entries.end(), _entries.begin(), _entries.end( ));
    }

    void process( detail::Socket& socket )
    {
        zmq_msg_t msg;
        zmq_msg_init( &msg );
        zmq_msg_recv( &msg, socket.socket, 0 );

        uint128_t type;
        memcpy( &type, zmq_msg_data( &msg ), sizeof(type) );
#ifndef ZEQ_LITTLEENDIAN
        detail::byteswap( type ); // convert from little endian wire
#endif
        const bool payload = zmq_msg_more( &msg );
        zmq_msg_close( &msg );

        zeq::Event event( type );
        if( payload )
        {
            zmq_msg_init( &msg );
            zmq_msg_recv( &msg, socket.socket, 0 );
            const size_t size = zmq_msg_size( &msg );
            ConstByteArray data( new uint8_t[size],
                                 std::default_delete< uint8_t[] >( ));
            memcpy( (void*)data.get(), zmq_msg_data( &msg ), size );
            event.setData( data, size );
            assert( event.getSize() == size );
            zmq_msg_close( &msg );
        }

        if( _eventFuncs.count( type ) != 0 )
            _eventFuncs[type]( event );
#ifndef NDEBUG
        else
        {
            // Note eile: The topic filtering in the handler registration should
            // ensure that we don't get messages we haven't handlers. If this
            // throws, something does not work.
            ZEQTHROW( std::runtime_error( "Got unsubscribed event" ));
        }
#endif
    }

    void update( void* context )
    {
        if( _service.isBrowsing( ))
            _service.browse( 0 );
        const servus::Strings& instances = _service.getInstances();

        for( const std::string& instance : instances )
        {
            const std::string& zmqURI = _getZmqURI( instance );

            // New subscription
            if( _subscribers.count( zmqURI ) == 0 )
            {
                if( !addConnection( context, zmqURI ))
                {
                    ZEQINFO << "Cannot connect subscriber to " << zmqURI << ": "
                            << zmq_strerror( zmq_errno( )) << std::endl;
                }
            }
        }
    }

    bool addConnection( void* context, const std::string& zmqURI )
    {
        _subscribers[zmqURI] = zmq_socket( context, ZMQ_SUB );

        if( zmq_connect( _subscribers[zmqURI], zmqURI.c_str( )) == -1 )
        {
            zmq_close( _subscribers[zmqURI] );
            _subscribers[zmqURI] = 0; // keep empty entry, unconnectable peer
            return false;
        }

        // Add existing subscriptions to socket
        for( const auto& i : _eventFuncs )
        {
            if( zmq_setsockopt( _subscribers[zmqURI], ZMQ_SUBSCRIBE,
                                &i.first, sizeof( uint128_t )) == -1 )
            {
                throw std::runtime_error(
                    std::string( "Cannot update topic filter: " ) +
                    zmq_strerror( zmq_errno( )));
            }
        }

        Socket entry;
        entry.socket = _subscribers[zmqURI];
        entry.events = ZMQ_POLLIN;
        _entries.push_back( entry );
        ZEQINFO << "Subscribed to " << zmqURI << std::endl;
        return true;
    }

private:
    std::string _getZmqURI( const std::string& instance )
    {
        const size_t pos = instance.find( ":" );
        const std::string& host = instance.substr( 0, pos );
        const std::string& port = instance.substr( pos + 1 );

        return buildZmqURI( host, std::stoi( port ));
    }

    typedef std::map< uint128_t, EventFunc > EventFuncs;
    typedef std::map< std::string, void* > SocketMap;

    SocketMap _subscribers;
    EventFuncs _eventFuncs;
    servus::Servus _service;
    std::vector< Socket > _entries;
};
}

Subscriber::Subscriber( const servus::URI& uri )
    : Receiver()
    , _impl( new detail::Subscriber( uri, getZMQContext( )))
{
}

Subscriber::Subscriber( const servus::URI& uri, Receiver& shared )
    : Receiver( shared )
    , _impl( new detail::Subscriber( uri, getZMQContext( )))
{
}

Subscriber::~Subscriber()
{
    delete _impl;
}

bool Subscriber::registerHandler( const uint128_t& event, const EventFunc& func)
{
    return _impl->registerHandler( event, func );
}

bool Subscriber::deregisterHandler( const uint128_t& event )
{
    return _impl->deregisterHandler( event );
}

void Subscriber::addSockets( std::vector< detail::Socket >& entries )
{
    _impl->addSockets( entries );
}

void Subscriber::process( detail::Socket& socket )
{
    _impl->process( socket );
}

void Subscriber::update()
{
    _impl->update( getZMQContext( ));
}

void Subscriber::addConnection( const std::string& uri )
{
    _impl->addConnection( getZMQContext(), uri );
}

}
