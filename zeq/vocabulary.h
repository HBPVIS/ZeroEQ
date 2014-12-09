
/* Copyright (c) 2014, Human Brain Project
 *                     Daniel Nachbaur <daniel.nachbaur@epfl.ch>
 *                     Juan Hernando <jhernando@fi.upm.es>
 */

#ifndef ZEQ_VOCABULARY_H
#define ZEQ_VOCABULARY_H

#include <zeq/api.h>
#include <zeq/types.h>

namespace zeq
{

/**
 * An application specific vocabulary of supported events including their
 * serialization. The implementation is dependend on a certain serialization
 * backend, which is flatbuffers by default. Events in the vocabulary are
 * identified by a 128 bit unique identifier, which should be created through
 * lunchbox::make_uint128() using a unique string formed using the fully
 * qualified class name, including namespaces, of the event, e.g.,
 * lunchbox::make_uint128( "mynamespace::ExcitingEvent" ).
 *
 * Example: @include tests/newEvent.fbs @include tests/newEvent.cpp
 */
namespace vocabulary
{
using lunchbox::make_uint128;

/** @group The supported event types by this vocabulary */
//@{
static const uint128_t EVENT_EXIT( make_uint128( "zeq::ExitEvent" ));
static const uint128_t EVENT_ECHO( make_uint128( "zeq::EchoEvent" ));
//@}

ZEQ_API Event serializeEcho( const std::string& message );
ZEQ_API std::string deserializeEcho( const Event& event );

}
}

#endif
