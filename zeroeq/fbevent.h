
/* Copyright (c) 2014-2016, Human Brain Project
 *                          Daniel Nachbaur <daniel.nachbaur@epfl.ch>
 */

#ifndef ZEROEQ_FBEVENT_H
#define ZEROEQ_FBEVENT_H

#include <zeroeq/api.h>
#include <zeroeq/types.h>

#include <servus/serializable.h>

namespace flatbuffers { class FlatBufferBuilder; class Parser; }

namespace zeroeq
{
namespace detail { class FBEvent; }

/**
 * An event is emitted by a Publisher to notify Subscriber of a change.
 *
 * Events are published via zeroeq::Publisher and received via zeroeq::Subscriber. The
 * format of the serialized data is specific to the serialization backend.
 *
 * Example: @include tests/hbp/serialization.cpp
 */
class FBEvent : public servus::Serializable
{
public:
    /**
     * Construct a new event of the given type
     *
     * @param type the desired event type
     * @param func the desired event function
     * @sa vocabulary::registerEvent
     */
    ZEROEQ_API FBEvent( const uint128_t& type, const FBEventFunc& func );

    /** Move ctor @internal */
    ZEROEQ_API FBEvent( FBEvent&& rhs );

    /** @return the universally unique identifier of this serializable. */
    ZEROEQ_API virtual uint128_t getTypeIdentifier() const;

    ZEROEQ_API ~FBEvent();

    /** @internal @return the size in bytes of the serialized data */
    ZEROEQ_API size_t getSize() const;

    /** @internal @return the serialized data */
    ZEROEQ_API const void* getData() const;

    /** @internal @return serialization specific implementation */
    ZEROEQ_API flatbuffers::FlatBufferBuilder& getFBB();

    /** @internal @return serialization specific implementation */
    ZEROEQ_API flatbuffers::Parser& getParser();

private:

    /**
     * @throw std::runtime_error because it is used by the REST
     * interface for http_server and this is not supported by FBEvent
     */
    ZEROEQ_API std::string getTypeName() const final;

    ZEROEQ_API bool _fromBinary( const void* data, const size_t size ) final;
    ZEROEQ_API Data _toBinary() const final;
    ZEROEQ_API bool _fromJSON( const std::string& json ) final;
    ZEROEQ_API std::string _toJSON() const final;

    std::unique_ptr< detail::FBEvent > _impl;
};

}

#endif
