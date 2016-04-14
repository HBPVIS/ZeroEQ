
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
ZEROEQ_API Event serializeEcho( const std::string& message );
ZEROEQ_API std::string deserializeEcho( const Event& event );

/**
 * Serialize the given vocabulary into an Event of type EVENT_VOCABULARY.
 * @param vocabulary the vocabulary ( a vector of EventDescriptors ).
 * @return the serialized event.
 */
ZEROEQ_API Event serializeVocabulary( const EventDescriptors& vocabulary );

/**
 * Deserialize the given EVENT_VOCABULARY event into a vector of EventDescriptors.
 * @param event the ZeroEQ EVENT_VOCABULARY.
 * @return the vector of EventDescriptors.
 */
ZEROEQ_API EventDescriptors deserializeVocabulary( const Event& event );


/** Serialize the given event type into an Event of type EVENT_REQUEST.
 *
 * Based on the type, the target application is responsible to publish the
 * requested event.
 *
 * @param eventType the type of event that the application should publish.
 * @return the serialized event.
 */
ZEROEQ_API Event serializeRequest( const uint128_t& eventType );

/** Deserialize the given request event into a uint128_t.
 *
 * @param event the ZeroEQ EVENT_REQUEST.
 * @return an uint128_t to identify the ZeroEQ event that should be published.
 */
ZEROEQ_API uint128_t deserializeRequest( const Event& event );
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
ZEROEQ_API void registerEvent( const uint128_t& type, const std::string& schema );

/** Serialize an event from JSON to a zeroeq::Event.
 *
 * The JSON must exactly match the schema of the given event type. The type to
 * schema matching is established through registerEvent() which usually happens
 * automatically for every known event that is present at compile time.

 * @sa registerEvent
 * @sa deserializeJSON
 * @param type the type of event to create
 * @param json JSON-formatted string containing the values for schema-defined
 *             keys
 * @return the serialized zeroeq::Event from the given JSON string.
 * @throw std::runtime_error when the parsing of the given JSON fails or the
 *        given event is not registered.
 */
ZEROEQ_API Event serializeJSON( const uint128_t& type, const std::string& json );

/** Deserialize a zeroeq::Event to a JSON string.
 *
 * In order to make this generic JSON deserialization, the type of the event
 * must be known to the vocabulary, which is usually done automatically already
 * by registerEvent().
 *
 * @sa registerEvent
 * @sa serializeJSON
 * @param event the zeroeq::Event to deserialize into JSON.
 * @return the deserialized JSON string from the given zeroeq::Event.
 * @throw std::runtime_error when the given event is not registered.
 */
ZEROEQ_API std::string deserializeJSON( const Event& event );
//@}

}
}

// must be after the declaration of registerEvent()
#include <zeroeq/echo_zeroeq_generated.h>
#include <zeroeq/exit_zeroeq_generated.h>
#include <zeroeq/heartbeat_zeroeq_generated.h>
#include <zeroeq/request_zeroeq_generated.h>
#include <zeroeq/vocabulary_zeroeq_generated.h>

#endif
