
/* Copyright (c) 2014-2016, Human Brain Project
 *                          Daniel Nachbaur <daniel.nachbaur@epfl.ch>
 *                          Juan Hernando <jhernando@fi.upm.es>
 */

#ifndef ZEROEQ_TYPES_H
#define ZEROEQ_TYPES_H

#include <zeroeq/defines.h>
#include <servus/types.h>
#include <servus/uint128_t.h>
#include <functional>
#include <memory>

#ifdef _WIN32
#  define NOMINMAX
#  include <winsock2.h> // SOCKET
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
namespace zeroeq
{

using servus::uint128_t;
class Event;
class Publisher;
class Subscriber;
class URI;

typedef std::function< void( const Event& ) > EventFunc;

#ifdef WIN32
typedef SOCKET SocketDescriptor;
#else
typedef int SocketDescriptor;
#endif

/** Constant defining 'wait forever' in methods with wait parameters. */
// Attn: identical to Win32 INFINITE!
static const uint32_t TIMEOUT_INDEFINITE = 0xffffffffu;

using servus::make_uint128;

static const std::string DEFAULT_SESSION = "__zeroeq";
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
