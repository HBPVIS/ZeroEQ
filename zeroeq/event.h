
/* Copyright (c) 2014-2016, Human Brain Project
 *                          Daniel Nachbaur <daniel.nachbaur@epfl.ch>
 */

#ifndef ZEROEQ_EVENT_H
#define ZEROEQ_EVENT_H

#include <zeroeq/api.h>
#include <zeroeq/types.h>

namespace zeroeq
{
namespace detail { class Event; }

/**
 * An event is emitted by a Publisher to notify Subscriber of a change.
 *
 * Events are published via zeroeq::Publisher and received via
 * zeroeq::Subscriber. The format of the serialized data is specific to the
 * serialization backend.
 *
 * Example: @include tests/serialization.cpp
 */
class Event
{
public:
    /**
     * Construct a new event of the given type
     *
     * @param type the desired event type
     * @sa vocabulary::registerEvent
     */
    ZEROEQ_API explicit Event( const uint128_t& type );

    /** Move ctor @internal */
    ZEROEQ_API Event( Event&& rhs );

    ZEROEQ_API ~Event();

    /** @return the type of this event */
    ZEROEQ_API const uint128_t& getType() const;

    /** @internal @return the size in bytes of the serialized data */
    ZEROEQ_API size_t getSize() const;

    /** @internal @return the serialized data */
    ZEROEQ_API const void* getData() const;

    /** @internal @return serialization specific implementation */
    ZEROEQ_API flatbuffers::FlatBufferBuilder& getFBB();

    /** @internal @return serialization specific implementation */
    ZEROEQ_API flatbuffers::Parser& getParser();

    /** @internal Set a raw buffer as event data. */
    void setData( const ConstByteArray& data, const size_t size );

private:
    Event( const Event& ) = delete;
    Event& operator=( const Event& ) = delete;

    Event& operator=( Event&& rhs );

    detail::Event* _impl;
};

}

#endif
