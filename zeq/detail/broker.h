
/* Copyright (c) 2014-2015, Human Brain Project
 *                          Daniel Nachbaur <daniel.nachbaur@epfl.ch>
 *                          Stefan.Eilemann@epfl.ch
 */

#ifndef ZEQ_DETAIL_BROKER_H
#define ZEQ_DETAIL_BROKER_H

#include "constants.h"

#include <zeq/uri.h>

#include <cstring>
#include <sstream>

// getlogin()
#ifdef _MSC_VER
#  include <sys/syscall.h>
#else
#  include <limits.h>
#  include <unistd.h>
#endif

namespace
{
std::string buildZmqURI( const std::string& schema,
                         std::string host, const uint16_t port )
{
    if( host.empty( ))
        host = "*";

    const std::string zmqURI( schema + "://" + host );
    if( port == 0 ) // zmq expects host:* instead of host:0
        return zmqURI + ":*";

    return zmqURI + ":" + std::to_string( int( port ));
}

std::string buildZmqURI( const zeq::URI& uri )
{
    return buildZmqURI( uri.getScheme(), uri.getHost(), uri.getPort( ));
}

std::string getUserName()
{
    const char* user = getlogin();
    return user ? user : UNKNOWN_USER;
}

std::string getDefaultSession()
{
    const char* session = getenv( ENV_SESSION.c_str( ));
    return session && strcmp(session, "") != 0 ? session : getUserName();
}

}

#endif
