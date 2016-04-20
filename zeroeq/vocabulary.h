
/* Copyright (c) 2014-2015, Human Brain Project
 *                          Daniel Nachbaur <daniel.nachbaur@epfl.ch>
 *                          Juan Hernando <jhernando@fi.upm.es>
 */

#ifndef ZEROEQ_VOCABULARY_H
#define ZEROEQ_VOCABULARY_H

#include <zeroeq/api.h>
#include <zeroeq/types.h>

namespace zeroeq
{

/**
 * An application specific vocabulary of supported events including their
 * serialization. The implementation is dependend on a certain serialization
 * backend, which is flatbuffers by default. Events in the vocabulary are
 * identified by a 128 bit unique identifier, which is automatically created by
 * the code generation of flatbuffers.
 *
 * Example: @include tests/newEvent.fbs @include tests/newEvent.cpp
 */
namespace vocabulary
{

/** @name Builtin Events */
//@{
ZEROEQ_API FBEvent serializeEcho( const std::string& message );
ZEROEQ_API std::string deserializeEcho( const FBEvent& event );
//@}

/**
 * Serialize a json to an event
 * @param json to serialize
 * @param type of the event
 * @param schema schema for the event
 * @return the event
 */
FBEvent serializeJSON( const std::string& json,
                       const uint128_t& type,
                       const std::string& schema );

/**
 * Deserialize a SCHEMA to a json
 * @param event is the event to be deserialized
 * @param schema is the schema for json
 * @return json
 */
std::string deserializeJSON( const FBEvent& event,
                             const std::string& schema );
}
}

// must be after the declaration of registerEvent()
#include <zeroeq/echo_zeroeq_generated.h>
#endif
