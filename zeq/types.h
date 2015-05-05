
/* Copyright (c) 2014-2015, Human Brain Project
 *                          Daniel Nachbaur <daniel.nachbaur@epfl.ch>
 */

#ifndef ZEQ_TYPES_H
#define ZEQ_TYPES_H

#include <lunchbox/types.h>
#include <lunchbox/uint128_t.h>
#include <functional>

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
struct EventDescriptor;

typedef std::vector< EventDescriptor > EventDescriptors;
typedef std::function< void( const Event& ) > EventFunc;

using lunchbox::uint128_t;
using lunchbox::make_uint128;

namespace detail { struct Socket; }

enum AnnounceMode //!< Network presence announcements
{
    ANNOUNCE_NONE = 0x0u, //!< Do not announce presence on the network
    ANNOUNCE_REQUIRED = 0x1u, //!< Chosen protocols are mandatory
    ANNOUNCE_ZEROCONF = 0x2u, //!< Force announcement using zeroconf
    ANNOUNCE_ALL = ANNOUNCE_ZEROCONF //!< Force announcement using all protocols
};

}
// internal
namespace flatbuffers { class FlatBufferBuilder; class Parser; }

#endif
