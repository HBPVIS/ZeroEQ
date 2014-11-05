
/* Copyright (c) 2014, Human Brain Project
 *                     Daniel Nachbaur <daniel.nachbaur@epfl.ch>
 *                     Stefan.Eilemann@epfl.ch
 */

#ifndef ZEQ_SUBSCRIBER_H
#define ZEQ_SUBSCRIBER_H

#include <boost/noncopyable.hpp>
#include <zeq/types.h>

namespace zeq
{

namespace detail { class Subscriber; }

/** Subscribes to Publisher to receive events. */
class Subscriber : public boost::noncopyable
{
public:
    /**
     * Create and subscribe to one or more publishers.
     *
     * A subscription to a non-existing publisher is valid. It will start
     * receiving events once the other publisher(s) is(are) publishing.
     *
     * @param uri publishing URI in the format scheme://[*|host|IP|IF][:port]
     * @throw std::runtime_error when the subscription failed.
     */
    explicit Subscriber( const lunchbox::URI& uri );

    /*
     * Create a shared subscription to one or more publishers.
     *
     * A receive on any Subscriber of a shared group will work on all
     * subscribers and call the registered handlers. Note to implementer: wrap
     * zmg_context in a shared_ptr object used by all instances.
     */
    //Subscriber( const lunchbox::URI& uri, const Subscriber& shared );

    /** Destroy this subscriber and withdraw any subscriptions. */
    ~Subscriber();

    /**
     * Receive one event from all connected publishers.
     *
     * For the received event, the respective handler function is called.
     *
     * @param timeout timeout in ms for poll, default blocking poll until at
     *                least one event is received
     * @return true if at least one event was received
     */
    bool receive( const uint32_t timeout = LB_TIMEOUT_INDEFINITE );

    /**
     * Register a new callback for an event.
     *
     * Only one callback per event is possible in the current implementation.
     *
     * @param event the event type of interest
     * @param func the callback function on receive of event
     * @return true if callback could be registered
     */
    bool registerHandler( const uint128_t& event, const EventFunc& func );

    /**
     * Deregister a callback for an event.
     *
     * @param event the event type of interest
     * @return true if callback could be deregistered
     */
    bool deregisterHandler( const uint128_t& event );

private:
    detail::Subscriber* const _impl;
};

}

#endif
