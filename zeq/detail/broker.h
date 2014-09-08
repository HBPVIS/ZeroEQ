
/* Copyright (c) 2014, Human Brain Project
 *                     Daniel Nachbaur <daniel.nachbaur@epfl.ch>
 *                     Stefan.Eilemann@epfl.ch
 */

#include <lunchbox/uri.h>
#include <sstream>

namespace
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

const std::string SERVICE_HOST( "zeq_host" );
const std::string SERVICE_PORT( "zeq_port" );
}
