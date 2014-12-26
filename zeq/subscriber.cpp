
/* Copyright (c) 2014, Human Brain Project
 *                     Daniel Nachbaur <daniel.nachbaur@epfl.ch>
 *                     Stefan.Eilemann@epfl.ch
 */

#include "subscriber.h"
#include "event.h"
#include "detail/broker.h"
#include "detail/socket.h"

#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <lunchbox/bitOperation.h>
#include <lunchbox/log.h>
#include <lunchbox/servus.h>
#include <map>

namespace zeq
{
namespace detail
{

class Subscriber
{
public:
    Subscriber( const lunchbox::URI& uri, void* context )
        : _service( std::string( "_" ) + uri.getScheme() + "._tcp" )
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
            update( context );
        }
        else
        {
            const std::string& zmqURI = buildZmqURI( uri );
            _addConnection( context, zmqURI );
        }
    }

    ~Subscriber()
    {
        BOOST_FOREACH( SocketType socket, _subscribers )
        {
            if ( socket.second )
                zmq_close( socket.second );
        }
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
#ifdef LB_BIGEENDIAN
        lunchbox::byteswap( type ); // convert from little endian wire
#endif
        const bool payload = zmq_msg_more( &msg );
        zmq_msg_close( &msg );

        zeq::Event event( type );
        if( payload )
        {
            zmq_msg_init( &msg );
            zmq_msg_recv( &msg, socket.socket, 0 );
            event.setData( zmq_msg_data( &msg ), zmq_msg_size( &msg ));
            zmq_msg_close( &msg );
        }

        if( _eventFuncs.count( type ) != 0 )
            _eventFuncs[type]( event );
    }

    void update( void* context )
    {
        _service.browse( 0 );
        const lunchbox::Strings& instances = _service.getInstances();

        BOOST_FOREACH( const std::string& instance, instances )
        {
            const std::string& zmqURI = _getZmqURI( instance );

            // New subscription
            if( _subscribers.count( zmqURI ) == 0 )
                _addConnection( context, zmqURI );
        }
    }

private:
    void _addConnection( void* context, const std::string& zmqURI )
    {
        _subscribers[zmqURI] = zmq_socket( context, ZMQ_SUB );

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

        Socket entry;
        entry.socket = _subscribers[zmqURI];
        entry.events = ZMQ_POLLIN;
        _entries.push_back( entry );
        LBINFO << "Subscribed to " << zmqURI << std::endl;
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

    SocketMap _subscribers;
    EventFuncs _eventFuncs;
    lunchbox::Servus _service;
    std::vector< Socket > _entries;
};
}

Subscriber::Subscriber( const lunchbox::URI& uri )
    : Receiver()
    , _impl( new detail::Subscriber( uri, getZMQContext( )))
{
}

Subscriber::Subscriber( const lunchbox::URI& uri, Receiver& shared )
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

}
