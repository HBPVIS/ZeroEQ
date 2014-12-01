
/* Copyright (c) 2014, Human Brain Project
 *                     Daniel Nachbaur <daniel.nachbaur@epfl.ch>
 *                     Stefan.Eilemann@epfl.ch
 */

// for NI_MAXHOST
#ifdef _WIN32
#define NOMINMAX
#  include <Ws2tcpip.h>
#else
#  include <netdb.h>
#endif

#include "subscriber.h"
#include "event.h"
#include "detail/broker.h"

#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <lunchbox/clock.h>
#include <lunchbox/log.h>
#include <lunchbox/servus.h>
#include <zmq.h>
#include <map>

namespace zeq
{
namespace detail
{

class Subscriber
{
public:
    Subscriber( const lunchbox::URI& uri )
        : _context( zmq_ctx_new( ))
        , _service( std::string( "_" ) + uri.getScheme() + "._tcp" )
    {
        if( uri.getScheme().empty( ))
            LBTHROW( std::runtime_error(
                         boost::lexical_cast< std::string >( uri ) +
                         " is not a valid URI (scheme is missing)."));

        if( uri.getHost().empty() || uri.getPort() == 0 )
        {
            if( !lunchbox::Servus::isAvailable( ))
                LBTHROW( std::runtime_error(
                             std::string( "Empty servus implementation" )));

            _service.beginBrowsing( lunchbox::Servus::IF_ALL );
            _refreshConnections();
        }
        else
        {
            const std::string& zmqURI = buildZmqURI( uri );
            _addConnection( zmqURI );
        }
    }

    ~Subscriber()
    {
        BOOST_FOREACH( SocketType socket, _subscribers )
        {
            if ( socket.second )
                zmq_close( socket.second );
        }
        zmq_ctx_destroy( _context );
    }

    bool receive( const uint32_t timeout )
    {
        if( _service.isBrowsing( ))
            return _receiveAndUpdate( timeout );
        return _receive( timeout );
    }
    bool registerHandler( const uint128_t& event, const EventFunc& func )
    {
        if( _eventFuncs.count( event ) != 0 )
            return false;
        _eventFuncs[event] = func;
        return true;
    }

    bool deregisterHandler( const uint128_t& event )
    {
        return _eventFuncs.erase( event ) > 0;
    }

private:
    void _refreshConnections()
    {
        _service.browse( 0 );
        const lunchbox::Strings& instances = _service.getInstances();

        BOOST_FOREACH( const std::string& instance, instances )
        {
            const std::string& zmqURI = _getZmqURI( instance );

            // New subscription
            if( _subscribers.count( zmqURI ) == 0 )
                _addConnection( zmqURI );
        }
    }

    void _addConnection( const std::string& zmqURI )
    {
        _subscribers[zmqURI] = zmq_socket( _context, ZMQ_SUB );

        if( zmq_connect( _subscribers[zmqURI], zmqURI.c_str( )) == -1 )
        {
            zmq_close( _subscribers[zmqURI] );
            _subscribers.erase( zmqURI );
            LBTHROW( std::runtime_error(
                         "Cannot connect subscriber to " + zmqURI + ", got " +
                         zmq_strerror( zmq_errno( ))));
        }

        if( zmq_setsockopt( _subscribers[zmqURI], ZMQ_SUBSCRIBE, "", 0 ) == -1 )
        {
            zmq_close( _subscribers[zmqURI] );
            _subscribers.erase( zmqURI );
            LBTHROW( std::runtime_error(
                         std::string( "Cannot set subscriber, got " ) +
                         zmq_strerror( zmq_errno( ))));
        }

        zmq_pollitem_t entry;
        entry.socket = _subscribers[zmqURI];
        entry.events = ZMQ_POLLIN;
        _entries.push_back( entry );
        LBINFO << "Subscribed to " << zmqURI << std::endl;
    }


    bool _receive( const uint32_t timeout )
    {
        switch( zmq_poll( _entries.data(), int(_entries.size( )),
                          timeout == LB_TIMEOUT_INDEFINITE ? -1 : timeout ))
        {
        case -1: // error
            LBTHROW( std::runtime_error( std::string( "Poll error: " ) +
                                         zmq_strerror( zmq_errno( ))));
        case 0: // timeout; no events signaled during poll
            return false;

        default:
            _process();
            return true;
        }
    }

    bool _receiveAndUpdate( const uint32_t timeout )
    {
        if( timeout == LB_TIMEOUT_INDEFINITE )
            return _receiveAndUpdate();

        // Never fully block. Check at least ten times or every second during a
        // receive for new connections from zeroconf (#20)
        const uint32_t block = std::min( 1000u, timeout / 10 );

        lunchbox::Clock timer;
        while( true )
        {
            _refreshConnections();

            const uint64_t elapsed = timer.getTime64();
            long wait = 0;
            if( elapsed < timeout )
                wait = std::min( timeout - uint32_t( elapsed ), block );

            if( _receive( wait ))
                return true;

            if( elapsed >= timeout )
                return false;
        }
    }

    bool _receiveAndUpdate()
    {
        while( true )
        {
            _refreshConnections();

            // Never fully block. Check every second during a receive for new
            // connections from zeroconf (#20)
            if( _receive( 1000 ))
                return true;
        }
    }

    void _process()
    {
        BOOST_FOREACH( const zmq_pollitem_t& entry, _entries )
        {
            if( !( entry.revents & ZMQ_POLLIN ))
                continue;

            zmq_msg_t msg;
            zmq_msg_init( &msg );
            zmq_msg_recv( &msg, entry.socket, 0 );

            uint128_t type;
            memcpy( &type, zmq_msg_data( &msg ), sizeof(type) );
#ifdef LB_BIGEENDIAN
            lunchbox::byteswap( type ); // convert from little endian wire
#endif
            const bool payload = zmq_msg_more( &msg );
            zmq_msg_close( &msg );

            zeq::Event event( type );
            if( payload )
            {
                zmq_msg_init( &msg );
                zmq_msg_recv( &msg, entry.socket, 0 );
                event.setData( zmq_msg_data( &msg ), zmq_msg_size( &msg ));
                zmq_msg_close( &msg );
            }

            if( _eventFuncs.count( type ) != 0 )
                _eventFuncs[type]( event );
        }
    }

    std::string _getZmqURI( const std::string& instance )
    {
        const size_t pos = instance.find( ":" );
        const std::string& host = instance.substr( 0, pos );
        const std::string& port = instance.substr( pos + 1 );

        return buildZmqURI( host, boost::lexical_cast< uint16_t >( port ));
    }

    typedef std::map< uint128_t, EventFunc > EventFuncs;
    typedef std::pair< std::string, void* > SocketType;
    typedef std::map< std::string, void* > SocketMap;

    void* _context;
    SocketMap _subscribers;
    EventFuncs _eventFuncs;
    lunchbox::Servus _service;
    std::vector< zmq_pollitem_t > _entries;
};
}

Subscriber::Subscriber( const lunchbox::URI& uri )
    : _impl( new detail::Subscriber( uri ))
{
}

Subscriber::~Subscriber()
{
    delete _impl;
}

bool Subscriber::receive( const uint32_t timeout )
{
    return _impl->receive( timeout );
}

bool Subscriber::registerHandler( const uint128_t& event, const EventFunc& func)
{
    return _impl->registerHandler( event, func );
}

bool Subscriber::deregisterHandler( const uint128_t& event )
{
    return _impl->deregisterHandler( event );
}

}
