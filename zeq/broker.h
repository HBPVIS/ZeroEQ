
/* Copyright (c) 2014, Human Brain Project
 *                     Daniel Nachbaur <daniel.nachbaur@epfl.ch>
 */

#ifndef ZEQ_BROKER_H
#define ZEQ_BROKER_H

#include <boost/noncopyable.hpp>
#include <zeq/types.h>

namespace zeq
{

namespace detail { class Broker; }

/**
 * The broker serves as publisher for events and can subscribe to other brokers
 * to receive events.
 */
class Broker : public boost::noncopyable
{
public:
    /**
     * Create a non-publishing broker
     */
    Broker();

    /**
     * Create a publishing broker on the given URI.
     *
     * @param uri publishing URI in the format scheme:://[*|IP|IF][:port|:*]
     */
    explicit Broker( const lunchbox::URI& uri );

    ~Broker();

    /**
     * Subscribe to the given broker URI to receive events.
     *
     * A subscription to a non-existing broker is valid. It will start receiving
     * events once the other broker is publishing.
     *
     * @param uri subscribing URI in the format scheme:://[host|IP|IF][:port|:*]
     * @return true if subscription was successful
     */
    bool subscribe( const lunchbox::URI& uri );

    /**
     * Unsubscribe from the broker URI to stop receive events.
     *
     * @param uri URI in the format scheme:://[*|host|IP][:port]
     * @return true if unsubscription was successful
     */
    bool unsubscribe( const lunchbox::URI& uri );

    /**
     * Publish the given event to any subscribed broker.
     *
     * If there is no subscriber for that event, no event will be send.
     *
     * @param event the serialized event to publish
     * @return true if publish was successful
     * @throw std::runtime_error if called on non-publishing broker
     */
    bool publish( const Event& event );

    /**
     * Receive once event for each subscriber.
     *
     * For each received event, the respective handler function is called.
     *
     * @param timeout timeout in ms for poll, default blocking
     * @return true if at least one event was received
     */
    bool receive( const uint32_t timeout = LB_TIMEOUT_INDEFINITE );

    /**
     * Register a callback for an event.
     *
     * Only one callback for one event is possible.
     *
     * @param event the event type of intereset
     * @param func the callback function on receive of event
     * @return true if callback can be registered
     */
    bool registerHandler( const uint64_t event, const EventFunc& func );

    /**
     * Deregister a callback for an event.
     *
     * @param event the event type of interest
     * @return true if callback can be deregistered
     */
    bool deregisterHandler( const uint64_t event );

private:
    detail::Broker* const _impl;
};

}

#endif
