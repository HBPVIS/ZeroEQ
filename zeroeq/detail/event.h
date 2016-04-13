
/* Copyright (c) 2014, Human Brain Project
 *                     Daniel Nachbaur <daniel.nachbaur@epfl.ch>
 */

#ifndef ZEROEQ_DETAIL_EVENT_H
#define ZEROEQ_DETAIL_EVENT_H

#include <zeroeq/types.h>
#include <zeroeq/event.h>
#include <zeroeq/log.h>

#include <flatbuffers/flatbuffers.h>
#include <flatbuffers/idl.h>

namespace zeroeq
{
namespace detail
{

namespace
{
static inline void dummy_deleter( const void* ) {}
}

class Event
{
public:
    explicit Event( const uint128_t& type_ )
        : type( type_ )
        , size( 0 )
    {}

    size_t getSize() const
    {
        if( data )
            return size;
        return parser.builder_.GetSize();
    }

    const void* getData() const
    {
        if( data )
            return data.get();
        return parser.builder_.GetBufferPointer();
    }

    void setData( const ConstByteArray& data_, const size_t size_ )
    {
        parser.builder_.Clear();
        data = data_;
        size = size_;
    }

    const uint128_t type;

    /** store the serialized data in here */
    flatbuffers::Parser parser;

    /** setData() uses this instead of fbb during deserialization */
    ConstByteArray data;
    size_t size;

private:
    Event( const Event& ) = delete;
    Event& operator=( const Event& ) = delete;
};

}
}

#endif
