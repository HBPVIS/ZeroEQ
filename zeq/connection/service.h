
/* Copyright (c) 2014-2015, Human Brain Project
 *                          Stefan.Eilemann@epfl.ch
 */

#ifndef ZEQ_CONNECTION_SERVICE_H
#define ZEQ_CONNECTION_SERVICE_H

#include <zeq/api.h>
#include <zeq/types.h>
#include <string>

namespace zeq
{
namespace connection
{

/**
 * Subscribes a Publisher to a remote receiver using a connection::Broker.
 *
 * Example: @include tests/connection/broker.cpp
 */
class Service
{
public:
    /**
     * Request subscription of the given publisher to a remote broker.
     *
     * Upon success, the Broker will add the publisher's address to its managed
     * Subscriber.
     *
     * @param address the broker address (hostname:port), without the protocol.
     * @param publisher the publisher to subscribe to.
     * @return true if the subscription was successful, false on error.
     */
    ZEQ_API static bool subscribe( const std::string& address,
                                   const Publisher& publisher );

    /**
     * Request subscription of the given publisher to a named remote broker.
     *
     * Upon success, the Broker will add the publisher's address to its managed
     * Subscriber. The broker port is derived using the same hashing algorithm
     * as in the corresponding Broker constructor.
     *
     * @param hostname the broker address, without the protocol and port.
     * @param name the application namespace.
     * @param publisher the publisher to subscribe to.
     * @return true if the subscription was successful, false on error.
     */
    ZEQ_API static bool subscribe( const std::string& hostname,
                                   const std::string& name,
                                   const Publisher& publisher );

private:
    Service( const Service& ) = delete;
    Service& operator=( const Service& ) = delete;
};

}
}
#endif
