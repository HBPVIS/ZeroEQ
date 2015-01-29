
/* Copyright (c) 2014, Human Brain Project
 *                     Daniel Nachbaur <daniel.nachbaur@epfl.ch>
 */

#ifndef ZEQ_TYPES_H
#define ZEQ_TYPES_H

#include <lunchbox/types.h>
#include <lunchbox/uint128_t.h>
#include <boost/function/function1.hpp>

/**
 * Publish-subscribe classes for typed events.
 *
 * A Publisher opens a listening port on the network, and publishes an Event on
 * this port. It announces its publishing URI scheme for automatic discovery.
 *
 * A Subscriber either explicitely subscribes to the publisher port, or uses
 * automatic discovery to find publishers using the same URI scheme. Automatic
 * discovery is implemented using zeroconf networking (avahi or Apple Bonjour).
 *
 * The connection::Broker and connection::Service may be used to introduce a
 * subscriber to a remote, not zeroconf visible, publisher.
 *
 * An Event contains a strongly type, semantically defined message. Applications
 * or groups of applications can define their own vocabulary.
 */
namespace zeq
{

class Event;
class Publisher;
class Subscriber;

typedef boost::function< void( const Event& ) > EventFunc;

using lunchbox::uint128_t;

namespace detail { struct Socket; }
}

// internal
namespace flatbuffers { class FlatBufferBuilder; }

#endif
