
/* Copyright (c) 2014-2016, Human Brain Project
 *                          Daniel Nachbaur <daniel.nachbaur@epfl.ch>
 *                          Stefan.Eilemann@epfl.ch
 */

#ifndef ZEQ_SUBSCRIBER_H
#define ZEQ_SUBSCRIBER_H

#include <zeq/receiver.h> // base class

#include <vector>

namespace zeq
{
/**
 * Subscribes to Publisher to receive events.
 *
 * If the subscriber is in the same session as discovered publishers, it
 * automatically subscribes to those publishers. Publishers from the same
 * application instance are not considered though.
 *
 * A subscription to a non-existing publisher is valid. It will start receiving
 * events once the other publisher(s) is(are) publishing.
 *
 * A receive on any Subscriber of a shared group will work on all subscribers
 * and call the registered handlers.
 *
 * Not thread safe.
 *
 * Example: @include tests/subscriber.cpp
 */
class Subscriber : public Receiver
{
public:
    /**
     * Create a default subscriber.
     *
     * Postconditions:
     * - discovers publishers on _zeroeq_pub._tcp ZeroConf service
     * - filters session \<username\> or ZEROEQ_SESSION from environment
     *
     * @throw std::runtime_error if ZeroConf is not available
     */
    ZEQ_API Subscriber();

    /**
     * Create a subscriber which subscribes to publisher(s) from the given
     * session.
     *
     * Postconditions:
     * - discovers publishers on _zeroeq_pub._tcp ZeroConf service
     * - filters for given session
     *
     * @param session session name used for filtering of discovered publishers
     * @throw std::runtime_error if ZeroConf is not available
     */
    ZEQ_API explicit Subscriber( const std::string& session );

    /**
     * Create a subscriber which subscribes to a specific publisher.
     *
     * Postconditions:
     * - connected to the publisher on the given URI once publisher is running
     *   on the URI
     *
     * @param uri publisher URI in the format [scheme://]*|host|IP|IF:port
     * @throw std::runtime_error if URI is not fully qualified
     */
    ZEQ_API explicit Subscriber( const URI& uri );

    /**
     * Create a subscriber which subscribes to publisher(s) on the given URI.
     *
     * The discovery and filtering by session is only used if the URI is not
     * fully qualified.
     *
     * Postconditions:
     * - discovers publishers on _zeroeq_pub._tcp ZeroConf service if URI is not
     *   fully qualified
     * - filters session \<username\> or ZEROEQ_SESSION from environment if
     *   zeq::DEFAULT_SESSION
     *
     * @param uri publisher URI in the format [scheme://][*|host|IP|IF][:port]
     * @param session session name used for filtering of discovered publishers
     * @throw std::runtime_error if ZeroConf is not available or if session name
     *                           is invalid
     */
    ZEQ_API Subscriber( const URI& uri, const std::string& session );

    /**
     * Create a default shared subscriber.
     *
     * @sa Subscriber()
     * @param shared another receiver to share data reception with
     */
    ZEQ_API Subscriber( Receiver& shared );

    /**
     * Create a shared subscriber which subscribes to publisher(s) from the
     * given session.
     *
     * @sa Subscriber( const std::string& )
     *
     * @param session only subscribe to publishers of the same session
     * @param shared another receiver to share data reception with
     */
    ZEQ_API Subscriber( const std::string& session, Receiver& shared );

    /**
     * Create a shared subscriber which subscribes to publisher(s) on the given
     * URI.
     *
     * @sa Subscriber( const URI& )
     *
     * @param uri publisher URI in the format [scheme://]*|host|IP|IF:port
     * @param shared another receiver to share data reception with
     */
    ZEQ_API Subscriber( const URI& uri, Receiver& shared );

    /**
     * Create a subscriber which subscribes to publisher(s) on the given URI.
     *
     * @sa Subscriber( const URI&, const std::string& )
     *
     * @param uri publisher URI in the format [scheme://][*|host|IP|IF][:port]
     * @param session session name used for filtering of discovered publishers
     * @param shared another receiver to share data reception with.
     */
    ZEQ_API Subscriber( const URI& uri, const std::string& session,
                        Receiver& shared );

    /** Destroy this subscriber and withdraw any subscriptions. */
    ZEQ_API ~Subscriber();

    /**
     * Register a new callback for an event.
     *
     * Only one callback per event is possible in the current implementation.
     *
     * @deprecated Use subscribe()
     * @param event the event type of interest
     * @param func the callback function on receive of event
     * @return true if callback could be registered
     */
    ZEQ_API bool registerHandler( const uint128_t& event,
                                  const EventFunc& func );

    /**
     * Deregister a callback for an event.
     *
     * @deprecated Use unsubscribe()
     * @param event the event type of interest
     * @return true if callback could be deregistered
     */
    ZEQ_API bool deregisterHandler( const uint128_t& event );

    /** @deprecated */
    ZEQ_API bool hasHandler( const uint128_t& event ) const;

    /**
     * Subscribe a serializable object to receive updates from any connected
     * publisher.
     *
     * Every update will be directly applied on the object during receive(). To
     * track updates on the object, the serializable's updated function is
     * called accordingly.
     *
     * The subscribed object instance has to be valid until unsubscribe().
     *
     * @param serializable the object to update on receive()
     * @return true if subscription was successful, false otherwise
     */
    ZEQ_API bool subscribe( servus::Serializable& serializable );

    /**
     * Unsubscribe a serializable object to stop applying updates from any
     * connected publisher.
     *
     * @param serializable the object to stop updating on receive()
     * @return true if removal of subscription was successful, false otherwise
     */
    ZEQ_API bool unsubscribe( const servus::Serializable& serializable );

    /** @return the session name that is used for filtering. */
    ZEQ_API const std::string& getSession() const;

private:
    class Impl;
    std::unique_ptr< Impl > _impl;

    // Receiver API
    void addSockets( std::vector< detail::Socket >& entries ) final;
    void process( detail::Socket& socket ) final;
    void update() final;
    void addConnection( const std::string& uri ) final;
};

}

#endif
