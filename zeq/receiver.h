
/* Copyright (c) 2014, Human Brain Project
 *                     Stefan.Eilemann@epfl.ch
 */

#ifndef ZEQ_RECEIVER_H
#define ZEQ_RECEIVER_H

#include <zeq/api.h>
#include <zeq/types.h>

#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

namespace zeq
{
namespace detail { class Receiver; }

/**
 * Base class for entities receiving data.
 * Not intended to be used independently. Not thread safe.
 */
class Receiver : public boost::noncopyable
{
public:
    /** Create a new standalone receiver. */
    ZEQ_API Receiver();

    /**
     * Create a shared receiver.
     *
     * All receivers sharing a group may receive data when receive() is called
     * on any of them.
     *
     * @param shared another receiver to share data reception with.
     */
    ZEQ_API explicit Receiver( Receiver& shared );

    /** Destroy this receiver. */
    ZEQ_API virtual ~Receiver();

    /**
     * Receive at least one event from all shared receivers.
     *
     * @param timeout timeout in ms for poll, default blocking poll until at
     *                least one event is received
     * @return true if at least one event was received
     * @throw std::runtime_error when polling failed.
     */
    ZEQ_API bool receive( const uint32_t timeout = LB_TIMEOUT_INDEFINITE );

protected:
    friend class detail::Receiver;

    /** Add this receiver's socket to the given list */
    virtual void addSockets( std::vector< detail::Socket >& entries ) = 0;

    /**
     * Process data on a signalled socket.
     *
     * @param the socket provided from addSockets().
     */
    virtual void process( detail::Socket& socket ) = 0;

    /**
     * Update the internal connection list.
     *
     * Called on all members of a shared group regularly by receive() to update
     * their list of sockets.
     */
    virtual void update() {}

    void* getZMQContext(); //!< @internal returns the ZeroMQ context

private:
    boost::shared_ptr< detail::Receiver > const _impl;
};

}

#endif
