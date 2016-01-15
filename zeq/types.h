
/* Copyright (c) 2014-2016, Human Brain Project
 *                          Daniel Nachbaur <daniel.nachbaur@epfl.ch>
 *                          Juan Hernando <jhernando@fi.upm.es>
 */

#ifndef ZEQ_TYPES_H
#define ZEQ_TYPES_H

#include <zeq/defines.h>
#include <servus/types.h>
#include <servus/uint128_t.h>
#include <functional>
#include <memory>

#ifdef ZEQ_USE_ZEROBUF
#  include <zerobuf/types.h>
#else
namespace zerobuf { class Schemas; } // dummy forward decl for vocabulary.h
#endif

/**
 * Publish-subscribe classes for typed events.
 *
 * A Publisher opens a listening port on the network, and publishes an Event on
 * this port. It announces its session for automatic discovery.
 *
 * A Subscriber either explicitely subscribes to the publisher port, or uses
 * automatic discovery to find publishers using the same session. Automatic
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

using servus::uint128_t;
class Event;
class Publisher;
class Subscriber;
class URI;
struct EventDescriptor;

typedef std::shared_ptr< const uint8_t > ConstByteArray;

typedef std::vector< EventDescriptor > EventDescriptors;
typedef std::function< void( const Event& ) > EventFunc;

/** Constant defining 'wait forever' in methods with wait parameters. */
// Attn: identical to Win32 INFINITE!
static const uint32_t TIMEOUT_INDEFINITE = 0xffffffffu;

using servus::make_uint128;

static const std::string DEFAULT_SESSION = "__zeq";
static const std::string NULL_SESSION = "__null_session";

namespace detail { struct Socket; }

/** @deprecated */
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
