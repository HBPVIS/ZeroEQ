
/* Copyright (c) 2014-2016, Human Brain Project
 *                          Daniel Nachbaur <daniel.nachbaur@epfl.ch>
 *                          Stefan.Eilemann@epfl.ch
 */

#ifndef ZEQ_PUBLISHER_H
#define ZEQ_PUBLISHER_H

#include <zeq/api.h>
#include <zeq/types.h>

#include <memory>

namespace zeq
{

/**
 * Serves and publishes events, consumed by Subscriber.
 *
 * The session is tied to ZeroConf announcement and can be disabled by passing
 * zeq::NULL_SESSION as the session name.
 *
 * Example: @include tests/publisher.cpp
 */
class Publisher
{
public:
    /**
     * Create a default publisher.
     *
     * Postconditions:
     * - bound to all network interfaces
     * - runs on a random port
     * - announces itself on the _zeroeq_pub._tcp ZeroConf service as host:port
     * - announces session \<username\> or ZEROEQ_SESSION from environment
     *
     * @throw std::runtime_error if session is empty or socket setup fails
     */
    ZEQ_API Publisher();

    /**
     * Create a publisher which announces the specified session.
     *
     * Postconditions:
     * - bound to all network interfaces
     * - runs on a random port
     * - announces itself on the _zeroeq_pub._tcp ZeroConf service as host:port
     * - announces given session
     *
     * @param session session name used for announcement
     * @throw std::runtime_error if session is empty or socket setup fails
     */
    ZEQ_API explicit Publisher( const std::string& session );

    /**
     * Create a publisher which runs on the specified URI.
     *
     * Postconditions:
     * - bound to the host and/or port from the given URI
     * - announces itself on the _zeroeq_pub._tcp ZeroConf service as host:port
     * - announces session \<username\> or ZEROEQ_SESSION from environment
     *
     * @param uri publishing URI in the format [scheme://][*|host|IP|IF][:port]
     * @throw std::runtime_error if session is empty or socket setup fails
     */
    ZEQ_API explicit Publisher( const URI& uri );

    /**
     * Create a publisher which runs on the specified URI and announces the
     * specified session.
     *
     * Postconditions:
     * - bound to the host and/or port from the given URI
     * - announces itself on the _zeroeq_pub._tcp ZeroConf service as host:port
     * - announces given session
     *
     * @param session session name used for announcement
     * @param uri publishing URI in the format [scheme://][*|host|IP|IF][:port]
     * @throw std::runtime_error if session is empty or socket setup fails
     */
    ZEQ_API Publisher( const URI& uri, const std::string& session );

    ZEQ_API ~Publisher();

    /**
     * Publish the given event to any subscriber.
     *
     * If there is no subscriber for that event, no event will be send.
     *
     * @deprecated Use publish() with serializable.
     * @param event the serialized event to publish
     * @return true if publish was successful
     */
    ZEQ_API bool publish( const Event& event );

    /**
     * Publish the given serializable object to any subscriber.
     *
     * If there is no subscriber for that serializable, no event will be sent.
     *
     * @param serializable the object to publish
     * @return true if publish was successful
     */
    ZEQ_API bool publish( const servus::Serializable& serializable );

    /**
     * Get the publisher URI.
     *
     * Contains the used hostname and port, if none where given in the
     * constructor uri.
     *
     * @return the publisher URI.
     * @todo change signature to return zeq::URI, needs downstream project
     *       adaptions. Also make zeq::URI( const servus::URI& from ) explicit.
     */
    ZEQ_API const servus::URI& getURI() const;

    /** @return the session name that is announced */
    ZEQ_API const std::string& getSession() const;

    ZEQ_API std::string getAddress() const; //!< @internal

private:
    class Impl;
    std::unique_ptr< Impl > _impl;

    Publisher( const Publisher& ) = delete;
    Publisher& operator=( const Publisher& ) = delete;
};

}

#endif
