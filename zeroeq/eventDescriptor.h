
/* Copyright (c) 2015, Human Brain Project
 *                     grigori.chevtchenko@epfl.ch
 */


#ifndef ZEROEQ_EVENTDESCRIPTOR_H
#define ZEROEQ_EVENTDESCRIPTOR_H

#include <zeroeq/api.h>
#include <zeroeq/types.h>

namespace zeroeq
{
namespace detail { struct EventDescriptor; }

/**
 * This enum holds information about the Event "direction" (only Published,
 * only Subscribed or Bidirectional) from the application point of view.
 */
enum EventDirection
{
    SUBSCRIBER = 0,
    PUBLISHER,
    BIDIRECTIONAL
};

/**
 * This structure holds informations about an Event from a vocabulary.
 * It contains the REST api name, the 128bit ZeroEQ event id and the schema
 * needed for automatic JSON serialization/deserialization.
 */
struct EventDescriptor
{
    /**
     * Create an EventDescriptor.
     * @param restName the string used for REST command
     * @param eventType the ZeroEQ event's uint128
     * @param schema the flatbuffers schema as string
     * @param eventDirection receiving, sending, or both
     */
    ZEROEQ_API EventDescriptor( const std::string& restName,
                                const uint128_t& eventType,
                                const std::string& schema,
                                const EventDirection eventDirection );

    /** Move ctor @internal */
    ZEROEQ_API EventDescriptor( EventDescriptor&& rhs );

    ZEROEQ_API ~EventDescriptor();

    /** @return the REST command string of this event */
    ZEROEQ_API const std::string& getRestName() const;

    /** @return the ZeroEQ event's uint128 */
    ZEROEQ_API const uint128_t& getEventType() const;

    /** @return the flatbuffers schema string*/
    ZEROEQ_API const std::string& getSchema() const;

    /** @return the ZeroEQ event's direction (Subscribed, Pulished or both) */
    ZEROEQ_API EventDirection getEventDirection() const;

private:
    EventDescriptor( const EventDescriptor& ) = delete;
    EventDescriptor& operator=( const EventDescriptor& ) = delete;

    EventDescriptor& operator=( EventDescriptor&& rhs );

    detail::EventDescriptor* _impl;
};

}

#endif
