
/* Copyright (c) 2015, Human Brain Project
 *                     Stefan.Eilemann@epfl.ch
 */

#ifndef ZEQ_DETAIL_SENDER_H
#define ZEQ_DETAIL_SENDER_H

#include <servus/uri.h>
#include <zmq.h>
#include <stdexcept>

// for NI_MAXHOST
#ifdef _WIN32
#  include <Ws2tcpip.h>
#else
#  include <unistd.h>
#  include <netdb.h>
#endif

namespace zeq
{
namespace detail
{

class Sender
{
    void* _context; // must be private before socket

public:
    Sender( void* context, const int type )
        : _context( 0 )
        , socket( zmq_socket( _createContext( context ), type ))
    {}

    Sender( const servus::URI& uri_, void* context, const int type )
        : _context( 0 )
        , uri( uri_ )
        , socket( zmq_socket( _createContext( context ), type ))
    {}

    ~Sender()
    {
        if( socket )
            zmq_close( socket );
        if( _context )
            zmq_ctx_destroy( _context );
    }

    std::string getAddress() const
    {
        return uri.getHost() + ":" + std::to_string( uint32_t( uri.getPort( )));
    }

    uint16_t getPort() const { return uri.getPort(); }

    void initURI()
    {
        std::string host = uri.getHost();
        if( host == "*" )
            host.clear();

        uint16_t port = uri.getPort();
        if( host.empty() || port == 0 )
        {
            std::string hostStr, portStr;
            _getEndPoint( hostStr, portStr );

            if( port == 0 )
            {
                // No overflow is possible unless ZMQ reports bad port number.
                port = std::stoi( portStr );
                uri.setPort( port );
            }

            if( host.empty( ))
                uri.setHost( hostStr );

            ZEQINFO << "Bound to " << uri << std::endl;
        }
    }

    servus::URI uri;
    void* socket;

private:
    void _getEndPoint( std::string& host, std::string& port ) const
    {
        char buffer[1024];
        size_t size = sizeof( buffer );
        if( zmq_getsockopt( socket, ZMQ_LAST_ENDPOINT, &buffer, &size ) == -1 )
        {
            ZEQTHROW( std::runtime_error(
                          "Cannot determine port of publisher" ));
        }
        const std::string endPoint( buffer );

        port = endPoint.substr( endPoint.find_last_of( ":" ) + 1 );
        const size_t start = endPoint.find_last_of( "/" ) + 1;
        const size_t end = endPoint.find_last_of( ":" );
        host = endPoint.substr( start, end - start );
        if( host == "0.0.0.0" )
        {
            char hostname[NI_MAXHOST+1] = {0};
            gethostname( hostname, NI_MAXHOST );
            hostname[NI_MAXHOST] = '\0';
            host = hostname;
        }
    }

    void* _createContext( void* context )
    {
        if( context )
            return context;

        _context = zmq_ctx_new();
        return _context;
    }
};

}
}

#endif
