
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

    const std::string tcpURI( "tcp://" + host );
    if( port == 0 ) // zmq expects host:* instead of host:0
        return tcpURI + ":*";

    return tcpURI + ":" + std::to_string( int( port ));
}

std::string buildZmqURI( const servus::URI& uri )
{
    return buildZmqURI( uri.getHost(), uri.getPort( ));
}

}
