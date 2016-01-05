
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

/** @name Builtin Events */
//@{
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


/** Serialize the given event type into an Event of type EVENT_REQUEST.
 *
 * Based on the type, the target application is responsible to publish the
 * requested event.
 *
 * @param eventType the type of event that the application should publish.
 * @return the serialized event.
 */
ZEQ_API Event serializeRequest( const uint128_t& eventType );

/** Deserialize the given request event into a uint128_t.
 *
 * @param event the zeq EVENT_REQUEST.
 * @return an uint128_t to identify the zeq event that should be published.
 */
ZEQ_API uint128_t deserializeRequest( const Event& event );

/** Serialize the given zerobuf::Schema into an Event. */
ZEQ_API Event serializeSchemas( const zerobuf::Schemas& schemas );

/** Deserialize the given event into zerobuf::Schema. */
ZEQ_API zerobuf::Schemas deserializeSchemas( const Event& event );
//@}

/**
 * @name JSON/binary event translation.
 *
 * These functions are not thread-safe, that is, calling registerEvent()
 * concurrently with any of the other functions in this group needs external
 * serialization.
 */
//@{
/** Establish a type to schema mapping for (de)serialization from/to JSON.
 *
 * @param type the type of the event.
 * @param schema the schema as string of the event.
 */
ZEQ_API void registerEvent( const uint128_t& type, const std::string& schema );

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
 * @throw std::runtime_error when the parsing of the given JSON fails or the
 *        given event is not registered.
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
 * @throw std::runtime_error when the given event is not registered.
 */
ZEQ_API std::string deserializeJSON( const Event& event );
//@}

}
}

// must be after the declaration of registerEvent()
#include <zeq/echo_zeq_generated.h>
#include <zeq/exit_zeq_generated.h>
#include <zeq/heartbeat_zeq_generated.h>
#include <zeq/request_zeq_generated.h>
#include <zeq/vocabulary_zeq_generated.h>

#endif
