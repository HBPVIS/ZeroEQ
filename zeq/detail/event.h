
/* Copyright (c) 2014, Human Brain Project
 *                     Daniel Nachbaur <daniel.nachbaur@epfl.ch>
 */

#ifndef ZEQ_DETAIL_EVENT_H
#define ZEQ_DETAIL_EVENT_H

#include <zeq/types.h>
#include <zeq/event.h>

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
        if( !data.empty( ))
            return data.size();
        return fbb.GetSize();
    }

    const void* getData() const
    {
        if( !data.empty( ))
            return data.data();
        return fbb.GetBufferPointer();
    }

    void setData( const void* data_, const size_t size )
    {
        fbb.Clear();
        data.resize( size );
        memcpy( data.data(), data_, size );
    }

    const uint128_t type;

    /** store the serialized data in here */
    flatbuffers::FlatBufferBuilder fbb;

    /** setData() uses this instead of fbb during deserialization */
    std::vector< uint8_t > data;
};

}
}

#endif
