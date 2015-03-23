
/* Copyright (c) 2014-2015, Human Brain Project
 *                          Stefan.Eilemann@epfl.ch
 */

#ifndef ZEQ_CONNECTION_BROKER_H
#define ZEQ_CONNECTION_BROKER_H

#include <zeq/receiver.h> // base class
#include <lunchbox/debug.h>

namespace zeq
{
/** Connection broker to introduce remote publishers to a subscriber. */
namespace connection
{
namespace detail { class Broker; }

/**
 * Brokers subscription requests for a zeq::Receiver.
 *
 * Example: @include tests/connection/broker.cpp
 */
class Broker : public Receiver
{
public:
    /**
     * Create a new subscription broker.
     *
     * The given receiver has to have at least the same lifetime as this
     * broker. The receiver and broker are automatically shared.
     *
     * For simplicity, only a single Receiver is handled by a Broker. The
     * implementation should be extended if multiple receivers shall be
     * handled.
     *
     * @param address the zmq reply socket address to be used.
     * @param receiver the Receiver to manage.
     * @throw std::runtime_error when the zmq setup failed.
     */
    ZEQ_API Broker( const std::string& address, Receiver& receiver );

    /** Destroy this broker. */
    ZEQ_API ~Broker();

private:
    detail::Broker* const _impl;

    // Receiver API
    void addSockets( std::vector< zeq::detail::Socket >& entries ) final;
    void process( zeq::detail::Socket& socket ) final;
    void addConnection( const std::string& ) final { LBDONTCALL; } // LCOV_EXCL_LINE
};

}
}
#endif
