
/* Copyright (c) 2014-2016, Human Brain Project
 *                          Daniel Nachbaur <daniel.nachbaur@epfl.ch>
 */

#ifndef ZEROEQ_EVENT_H
#define ZEROEQ_EVENT_H

#include <zeroeq/api.h>
#include <zeroeq/types.h>

#include <servus/serializable.h>

namespace zeroeq
{
namespace detail { class Event; }

/**
 * An event is emitted by a Publisher to notify Subscriber of a change.
 *
 * Events are published via zeroeq::Publisher and received via zeroeq::Subscriber. The
 * format of the serialized data is specific to the serialization backend.
 *
 * Example: @include tests/serialization.cpp
 */
class Event : public servus::Serializable
{
public:
    /**
     * Construct a new event of the given type
     *
     * @param type the desired event type
     * @param func the desired event function
     * @sa vocabulary::registerEvent
     */
    ZEROEQ_API Event( const uint128_t& type, const EventFunc& func = EventFunc( ));

    /** Move ctor @internal */
    ZEROEQ_API Event( Event&& rhs );

    /** @return the fully qualified, demangled class name. */
    ZEROEQ_API std::string getTypeName() const final;

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

private:

    ZEROEQ_API bool _fromBinary( const void* data, const size_t size ) final;
    ZEROEQ_API Data _toBinary() const final;
    ZEROEQ_API bool _fromJSON( const std::string& json ) final;
    ZEROEQ_API std::string _toJSON() const final;

    std::unique_ptr< detail::Event > _impl;
};

}

#endif
