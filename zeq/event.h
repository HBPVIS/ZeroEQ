
/* Copyright (c) 2014, Human Brain Project
 *                     Daniel Nachbaur <daniel.nachbaur@epfl.ch>
 */

#ifndef ZEQ_EVENT_H
#define ZEQ_EVENT_H

#include <zeq/types.h>

namespace zeq
{
namespace detail { class Subscriber; class Event; }

/**
 * An event can notify other brokers of a state change.
 *
 * Events are published and received via the zeq::Broker. The format of the
 * serialized data is specific to the serialization backend.
 */
class Event : public boost::noncopyable
{
public:
    /**
     * Construct a new event of the given type
     * @param type the desired event type
     */
    explicit Event( const uint64_t type );

    /** Move ctor @internal */
    Event( Event&& rhs );

    ~Event();

    /** @return the type of this event */
    uint64_t getType() const;

    /** @internal @return the size in bytes of the serialized data */
    size_t getSize() const;

    /** @internal @return the serialized data */
    const void* getData() const;

    /** @internal @return serialization specific implementation */
    detail::Event* getImpl();

private:
    friend class detail::Subscriber;
    void setData( const void* data, const size_t size );

    Event& operator=( Event&& rhs );

    detail::Event* _impl;
};

}

#endif
