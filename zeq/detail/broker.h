
/* Copyright (c) 2014, Human Brain Project
 *                     Daniel Nachbaur <daniel.nachbaur@epfl.ch>
 *                     Stefan.Eilemann@epfl.ch
 */

#include <servus/uri.h>

#include <sstream>

namespace
{
std::string buildZmqURI( std::string host, const uint16_t port )
{
    if( host.empty( ))
        host = "*";
    if( host != "*" && port == 0 ) // zmq does not support host:0
        throw std::runtime_error(
            "OS-chosen port not supported with hostname " + host );

    return std::string( "tcp://" ) + host + ":" + std::to_string( int( port ));
}

std::string buildZmqURI( const servus::URI& uri )
{
    return buildZmqURI( uri.getHost(), uri.getPort( ));
}

}
