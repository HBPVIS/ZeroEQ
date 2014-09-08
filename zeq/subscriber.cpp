
/* Copyright (c) 2014, Human Brain Project
 *                     Daniel Nachbaur <daniel.nachbaur@epfl.ch>
 *                     Stefan.Eilemann@epfl.ch
 */

#include "subscriber.h"
#include "event.h"
#include "detail/broker.h"

#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
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

namespace zeq
{
namespace detail
{

class Subscriber
{
public:
    Subscriber( const lunchbox::URI& uri )
        : _context( zmq_ctx_new( ))
        , _subscriber( zmq_socket( _context, ZMQ_SUB ))
        , _service( std::string( "_" ) + uri.getScheme() + "._tcp" )
    {
        // TODO: continuous browsing and update with zeroconf uris. One
        // subscriber will receive from multiple publishers, i.e., we need to
        // manage and update multiple sockets.
        const std::string& zmqURI = _buildSubscriberURI( uri );
        if( zmqURI.empty( ))
        {
            LBTHROW( std::runtime_error(
                         "Could not find a (suitable) publisher for " +
                         boost::lexical_cast< std::string >( uri )));
        }
        if( zmq_connect( _subscriber, zmqURI.c_str( )) == -1 )
        {
            zmq_close( _subscriber );
            _subscriber = 0;
            LBTHROW( std::runtime_error(
                         "Cannot connect subscriber to " + zmqURI + ", got " +
                         zmq_strerror( zmq_errno( ))));
        }

        if( zmq_setsockopt( _subscriber, ZMQ_SUBSCRIBE, "", 0 ) == -1 )
        {
            zmq_close( _subscriber );
            _subscriber = 0;
            LBTHROW( std::runtime_error(
                         std::string( "Cannot set subscriber, got " ) +
                         zmq_strerror( zmq_errno( ))));
        }
    }

    ~Subscriber()
    {
        if( _subscriber )
            zmq_close( _subscriber );
        zmq_ctx_destroy( _context );
    }

    bool receive( const uint32_t timeout )
    {
        std::vector< zmq_pollitem_t > entries( 1 );
        zmq_pollitem_t &entry = entries[0];
        entry.socket = _subscriber;
        entry.events = ZMQ_POLLIN;

        const int iPoll = zmq_poll( entries.data(), entries.size(),
                                    timeout == LB_TIMEOUT_INDEFINITE ? -1
                                                                     : timeout);
        if( iPoll == -1 )
        {
            LBWARN << "Cannot poll, got " << zmq_strerror( zmq_errno( ))
                   << std::endl;
            return false;
        }

        if( iPoll == 0 || !( entries[0].revents & ZMQ_POLLIN ))
            return false;

        zmq_msg_t msg;
        zmq_msg_init( &msg );
        zmq_msg_recv( &msg, entries[0].socket, 0 );

        uint64_t type;
        memcpy( &type, zmq_msg_data( &msg ), sizeof(type) );

        const bool more = zmq_msg_more( &msg );
        zmq_msg_close( &msg );
        if( !more )
            return false;

        zmq_msg_init( &msg );
        zmq_msg_recv( &msg, entries[0].socket, 0 );
        zeq::Event event( type );
        event.setData( zmq_msg_data( &msg ), zmq_msg_size( &msg ));
        zmq_msg_close( &msg );

        if( _eventFuncs.count( type ) != 0 )
            _eventFuncs[type]( event );

        return true;
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
    std::string _buildSubscriberURI( const lunchbox::URI& uri )
    {
        if( !uri.getHost().empty() && uri.getPort() != 0 )
            return buildZmqURI( uri );

        const lunchbox::Strings& instances =
                _service.discover( lunchbox::Servus::IF_ALL, 500 );
        BOOST_FOREACH( const std::string& instance, instances )
        {
            const size_t pos = instance.find( ":" );
            const std::string& host = instance.substr( 0, pos );
            const std::string& port = instance.substr( pos + 1 );
            return buildZmqURI( host, boost::lexical_cast< uint16_t >( port ));
        }
        return std::string();
    }

    typedef std::map< uint64_t, EventFunc > EventFuncs;

    void* _context;
    void* _subscriber;
    EventFuncs _eventFuncs;
    lunchbox::Servus _service;
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

bool Subscriber::registerHandler( const uint64_t event, const EventFunc& func )
{
    return _impl->registerHandler( event, func );
}

bool Subscriber::deregisterHandler( const uint64_t event )
{
    return _impl->deregisterHandler( event );
}

}
