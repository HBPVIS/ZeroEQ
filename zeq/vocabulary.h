
/* Copyright (c) 2014-2015, Human Brain Project
 *                          Daniel Nachbaur <daniel.nachbaur@epfl.ch>
 *                          Juan Hernando <jhernando@fi.upm.es>
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
 * identified by a 128 bit unique identifier, which is automatically created by
 * the code generation of flatbuffers.
 *
 * Example: @include tests/newEvent.fbs @include tests/newEvent.cpp
 */
namespace vocabulary
{

ZEQ_API Event serializeEcho( const std::string& message );
ZEQ_API std::string deserializeEcho( const Event& event );

/**
 * Serialize the given vocabulary into an Event of type EVENT_VOCABULARY.
 * @param vocabulary the vocabulary ( a vector of EventDescriptors ).
 * @return the serialized event.
 */
ZEQ_API Event serializeVocabulary( const EventDescriptors& vocabulary );

/**
 * Deserialize the given EVENT_VOCABULARY event into a vector of EventDescriptors.
 * @param event the zeq EVENT_VOCABULARY.
 * @return the vector of EventDescriptors.
 */
ZEQ_API EventDescriptors deserializeVocabulary( const Event& event );


/** Serialize an event from JSON to a zeq::Event.
 *
 * The JSON must exactly match the schema of the given event type. The type to
 * schema matching is established through registerEvent() which usually happens
 * automatically for every known event that is present at compile time.

 * @sa registerEvent
 * @sa deserializeJSON
 * @param type the type of event to create
 * @param json JSON-formatted string containing the values for schema-defined
 *             keys
 * @return the serialized zeq::Event from the given JSON string.
 */
ZEQ_API Event serializeJSON( const uint128_t& type, const std::string& json );

/** Deserialize a zeq::Event to a JSON string.
 *
 * In order to make this generic JSON deserialization, the type of the event
 * must be known to the vocabulary, which is usually done automatically already
 * by registerEvent().
 *
 * @sa registerEvent
 * @sa serializeJSON
 * @param event the zeq::Event to deserialize into JSON.
 * @return the deserialized JSON string from the given zeq::Event.
 */
ZEQ_API std::string deserializeJSON( const Event& event );

/** Establish a type to schema mapping for (de)serialization from/to JSON.
 *
 * @param type the type of the event.
 * @param schema the schema as string of the event.
 */
ZEQ_API void registerEvent( const uint128_t& type, const std::string& schema );

}
}

#include <zeq/echo_zeq_generated.h>
#include <zeq/exit_zeq_generated.h>
#include <zeq/vocabulary_zeq_generated.h>

#endif
