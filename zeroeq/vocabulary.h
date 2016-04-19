
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
}
}

// must be after the declaration of registerEvent()
#include <zeroeq/echo_zeroeq_generated.h>
#include <zeroeq/exit_zeroeq_generated.h>
#endif
