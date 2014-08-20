
/* Copyright (c) 2014, Human Brain Project
 *                     Daniel Nachbaur <daniel.nachbaur@epfl.ch>
 */

#include "broker.h"
#include "event.h"

#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <lunchbox/log.h>
#include <lunchbox/servus.h>
#include <lunchbox/uri.h>
#include <map>
#include <zmq.h>

// for NI_MAXHOST
#ifdef _WIN32
#  include <Ws2tcpip.h>
#else
#  include <netdb.h>
#endif

namespace zeq
{
namespace detail
{

std::string buildZmqURI( const std::string& host, const uint16_t port )
{
    std::ostringstream zmqURI;
    zmqURI << "tcp://";
    if( host.empty( ))
        zmqURI << "*";
    else
        zmqURI << host;
    if( port == 0 )
        zmqURI << ":*";
    else
        zmqURI << ":" << port;
    return zmqURI.str();
}

std::string buildZmqURI( const lunchbox::URI& uri )
{
    return buildZmqURI( uri.getHost(), uri.getPort( ));
}

const std::string SERVICE_TYPE( "zeq_type" );
const std::string SERVICE_HOST( "zeq_host" );
const std::string SERVICE_PORT( "zeq_port" );

class Broker
{
public:
    Broker()
        : _context( zmq_ctx_new( ))
        , _publisher( 0 )
        , _service( "_zeq._tcp" )
    {
    }

    Broker( const lunchbox::URI& uri )
        : _context( zmq_ctx_new( ))
        , _publisher( zmq_socket( _context, ZMQ_PUB ))
        , _service( "_zeq._tcp" )
    {
        if( zmq_bind( _publisher, buildZmqURI( uri ).c_str( )) == -1 )
        {
            zmq_close( _publisher );
            zmq_ctx_destroy( _context );

            LBTHROW( std::runtime_error(
                         std::string( "Cannot bind publisher socket, got " ) +
                                      zmq_strerror( zmq_errno( ))));
        }

        _initService( uri.getScheme(), uri.getHost(), uri.getPort( ));
    }

    ~Broker()
    {
        BOOST_FOREACH( const SubscriberMap::value_type& entry, _subscribers )
            zmq_close( entry.second );
        zmq_close( _publisher );
        zmq_ctx_destroy( _context );
    }

    bool subscribe( const lunchbox::URI& uri )
    {
        const std::string& uriString = boost::lexical_cast< std::string >( uri );
        if( _subscribers.count( uriString ) != 0 )
            return false;

        const std::string& zmqURI = _buildSubscriberURI( uri );
        if( zmqURI.empty( ))
        {
            LBWARN << "Could not find a (suitable) publisher for " << uriString
                   << std::endl;
            return false;
        }
        void* subscriber = zmq_socket( _context, ZMQ_SUB );
        if( zmq_connect( subscriber, zmqURI.c_str( )) == -1 )
        {
            LBWARN << "Cannot connect subscriber to " << zmqURI << ", got "
                   << zmq_strerror( zmq_errno( )) << std::endl;
            zmq_close( subscriber );
            return false;
        }

        if( zmq_setsockopt( subscriber, ZMQ_SUBSCRIBE, "", 0 ) == -1 )
        {
            LBWARN << "Cannot set subscriber, got "
                   << zmq_strerror( zmq_errno( )) << std::endl;
            zmq_close( subscriber );
            return false;
        }

        _subscribers[uriString] = subscriber;
        return true;
    }

    bool unsubscribe( const lunchbox::URI& uri )
    {
        const std::string& uriString = boost::lexical_cast< std::string >( uri );
        if( _subscribers.count( uriString ) == 0 )
            return false;
        zmq_close( _subscribers[uriString] );
        _subscribers.erase( uriString );
        return true;
    }

    bool publish( const zeq::Event& event )
    {
        if( !_publisher )
            LBTHROW( std::runtime_error(
                         "Cannot publish on a non-publishing broker" ));

        const uint64_t type = event.getType();
        zmq_msg_t msgHeader;
        zmq_msg_init_size( &msgHeader, sizeof(type));
        memcpy( zmq_msg_data(&msgHeader), &type, sizeof(type));

        zmq_msg_t msg;
        zmq_msg_init_size( &msg, event.getSize( ));
        memcpy( zmq_msg_data(&msg), event.getData(), event.getSize( ));

        if( zmq_msg_send( &msgHeader, _publisher, ZMQ_SNDMORE ) == -1 ||
            zmq_msg_send( &msg, _publisher, 0 ) == -1 )
        {
            zmq_msg_close( &msgHeader );
            zmq_msg_close( &msg );
            LBWARN << "Cannot publish, got " << zmq_strerror( zmq_errno( ))
                   << std::endl;
            return false;
        }
        zmq_msg_close( &msgHeader );
        zmq_msg_close( &msg );
        return true;
    }

    bool receive( const uint32_t timeout )
    {
        std::vector< zmq_pollitem_t > entries( _subscribers.size( ));
        size_t i = 0;
        BOOST_FOREACH( const SubscriberMap::value_type& sub, _subscribers )
        {
            zmq_pollitem_t &entry = entries[i++];
            entry.socket = sub.second;
            entry.events = ZMQ_POLLIN;
        }

        const int iPoll = zmq_poll( entries.data(), _subscribers.size(),
                                    timeout == LB_TIMEOUT_INDEFINITE ? -1
                                                                     : timeout);

        if( iPoll == -1 )
        {
            LBWARN << "Cannot poll, got " << zmq_strerror( zmq_errno( ))
                   << std::endl;
            return false;
        }

        if( iPoll == 0 )
            return false;

        bool hasEvent = false;
        for( i = 0; i < _subscribers.size(); ++i )
        {
            if( entries[i].revents & ZMQ_POLLIN )
            {
                zmq_msg_t msg;
                zmq_msg_init( &msg );
                zmq_msg_recv( &msg, entries[i].socket, 0 );

                uint64_t type;
                memcpy( &type, zmq_msg_data( &msg ), sizeof(type) );
                zmq_msg_close( &msg );

                if( !zmq_msg_more( &msg ))
                    return false;

                zmq_msg_init( &msg );
                zmq_msg_recv( &msg, entries[i].socket, 0 );
                zeq::Event event( type );
                event.setData( zmq_msg_data( &msg ), zmq_msg_size( &msg ));
                zmq_msg_close( &msg );

                if( _eventFuncs.count( type ) != 0 )
                    _eventFuncs[type]( event );

                hasEvent = true;
            }
        }

        return hasEvent;
    }

    bool registerHandler( const uint64_t event, const EventFunc& func )
    {
        if( _eventFuncs.count( event ) != 0 )
            return false;
        _eventFuncs[event] = func;
        return true;
    }

    bool deregisterHandler( const uint64_t event )
    {
        return _eventFuncs.erase( event ) > 0;
    }

private:
    void _initService( const std::string& type, std::string host, uint16_t port)
    {
        if( !_publisher )
            return;

        if( host == "*" )
            host.clear();

        if( host.empty() || port == 0 )
            _resolveHostAndPort( host, port );

        _service.withdraw(); // go silent during k/v update
        _service.set( SERVICE_TYPE, type );
        _service.set( SERVICE_HOST, host );
        _service.set( SERVICE_PORT, boost::lexical_cast< std::string >( port ));
        _service.announce( port, host );
    }

    void _resolveHostAndPort( std::string& host, uint16_t& port )
    {
        char endPoint[1024];
        size_t size = sizeof(endPoint);
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
            port = boost::lexical_cast< uint16_t >( portStr );
        }

        if( host.empty( ))
        {
            host = endPointStr.substr( endPointStr.find_last_of( "/" ) + 1 );
            host = host.substr( 0, host.size() - host.find_last_of( ":" ) + 1 );
            if( host == "0.0.0.0" )
            {
                char hostname[NI_MAXHOST+1] = {0};
                gethostname( hostname, NI_MAXHOST );
                hostname[NI_MAXHOST] = '\0';
                host = hostname;
            }
        }
    }

    std::string _buildSubscriberURI( const lunchbox::URI& uri )
    {
        if( !uri.getHost().empty() && uri.getPort() != 0 )
            return buildZmqURI( uri );

        const lunchbox::Strings& instances =
                _service.discover( lunchbox::Servus::IF_ALL, 500 );
        BOOST_FOREACH( const std::string& instance, instances )
        {
            const std::string& type = _service.get( instance, SERVICE_TYPE );
            if( type != uri.getScheme( ))
                continue;

            const std::string& host = _service.get( instance, SERVICE_HOST );
            const std::string& port = _service.get( instance, SERVICE_PORT );
            return buildZmqURI( host.empty() ? instance : host,
                                boost::lexical_cast< uint16_t >( port ));
        }
        return std::string();
    }

    typedef std::map< std::string, void* > SubscriberMap;
    typedef std::map< uint64_t, EventFunc > EventFuncs;

    void* _context;
    void* _publisher;
    SubscriberMap _subscribers;
    EventFuncs _eventFuncs;
    lunchbox::Servus _service;
};
}

Broker::Broker()
    : _impl( new detail::Broker )
{
}

Broker::Broker( const lunchbox::URI& uri )
    : _impl( new detail::Broker( uri ))
{
}

Broker::~Broker()
{
    delete _impl;
}

bool Broker::subscribe( const lunchbox::URI& uri )
{
    return _impl->subscribe( uri );
}

bool Broker::unsubscribe( const lunchbox::URI& uri )
{
    return _impl->unsubscribe( uri );
}

bool Broker::publish( const Event& event )
{
    return _impl->publish( event );
}

bool Broker::receive( const uint32_t timeout )
{
    return _impl->receive( timeout );
}

bool Broker::registerHandler( const uint64_t event, const EventFunc& func )
{
    return _impl->registerHandler( event, func );
}

bool Broker::deregisterHandler( const uint64_t event )
{
    return _impl->deregisterHandler( event );
}

}
