
/* Copyright (c) 2014, Human Brain Project
 *                     Daniel Nachbaur <daniel.nachbaur@epfl.ch>
 */

#ifndef ZEQ_DETAIL_EVENT_H
#define ZEQ_DETAIL_EVENT_H

#include <zeq/types.h>
#include <zeq/event.h>

#include <lunchbox/buffer.h>
#include <flatbuffers/flatbuffers.h>

namespace zeq
{
namespace detail
{

class Event : public boost::noncopyable
{
public:
    Event( const uint128_t& type_ )
        : type( type_ )
    {}

    size_t getSize() const
    {
        if( !data.isEmpty( ))
            return data.getSize();
        return fbb.GetSize();
    }

    const void* getData() const
    {
        if( !data.isEmpty( ))
            return data.getData();
        return fbb.GetBufferPointer();
    }

    void setData( const void* data_, const size_t size )
    {
        fbb.Clear();
        data.replace( data_, size );
    }

    const uint128_t type;

    /** store the serialized data in here */
    flatbuffers::FlatBufferBuilder fbb;

    /** setData() uses this instead of fbb during deserialization */
    lunchbox::Bufferb data;
};

}
}

#endif
