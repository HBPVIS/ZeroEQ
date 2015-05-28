
/* Copyright (c) 2014-2015, Human Brain Project
 *                          Stefan.Eilemann@epfl.ch
 */

#include "service.h"
#include <zeq/publisher.h>
#include <zeq/detail/port.h>
#include <zeq/log.h>

#include <zmq.h>

#include <string.h>

namespace zeq
{
namespace connection
{
bool Service::subscribe( const std::string& brokerAddress,
                         const Publisher& publisher )
{
    void* context = zmq_ctx_new();
    void* socket = zmq_socket( context, ZMQ_REQ );
    const std::string zmqAddress = std::string("tcp://" ) + brokerAddress;
    if( zmq_connect( socket, zmqAddress.c_str( )) == -1 )
    {
        ZEQINFO << "Can't reach connection broker at " << brokerAddress
                << std::endl;
        zmq_close( socket );
        zmq_ctx_destroy( context );
        return false;
    }

    const std::string& address = publisher.getAddress();
    zmq_msg_t request;
    zmq_msg_init_size( &request, address.size( ));
    memcpy( zmq_msg_data( &request ), address.c_str(), address.size( ));

    if( zmq_msg_send( &request, socket, 0 ) == -1 )
    {
        zmq_msg_close( &request );
        ZEQINFO << "Can't send connection request " << address << " to "
                << brokerAddress << ": " << zmq_strerror( zmq_errno( ))
                << std::endl;
        return false;
    }
    zmq_msg_close( &request );

    zmq_msg_t reply;
    zmq_msg_init( &reply );
    if( zmq_msg_recv( &reply, socket, 0 )  == -1 )
    {
        zmq_msg_close( &reply );
        ZEQINFO << "Can't receive connection reply from " << brokerAddress
                << std::endl;
        return false;
    }

    const std::string result( (const char*)zmq_msg_data( &reply ),
                              zmq_msg_size( &reply ));
    zmq_msg_close( &reply );

    zmq_close( socket );
    zmq_ctx_destroy( context );

    return address == std::string( result );
}

bool Service::subscribe( const std::string& brokerAddress,
                         const std::string& name,
                         const Publisher& publisher )
{
    const std::string address( brokerAddress + ":" +
                            std::to_string( uint32_t(detail::getPort( name ))));
    return subscribe( address, publisher );
}

}
}
