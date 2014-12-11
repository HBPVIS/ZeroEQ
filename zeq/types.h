
/* Copyright (c) 2014, Human Brain Project
 *                     Daniel Nachbaur <daniel.nachbaur@epfl.ch>
 */

#ifndef ZEQ_TYPES_H
#define ZEQ_TYPES_H

#include <lunchbox/types.h>
#include <lunchbox/uint128_t.h>
#include <boost/function/function1.hpp>

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
