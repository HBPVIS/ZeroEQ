
/* Copyright (c) 2014, Human Brain Project
 *                     Daniel Nachbaur <daniel.nachbaur@epfl.ch>
 */

#ifndef ZEROEQ_DETAIL_EVENT_H
#define ZEROEQ_DETAIL_EVENT_H

#include <zeroeq/types.h>
#include <zeroeq/fbevent.h>
#include <zeroeq/log.h>

#include <flatbuffers/flatbuffers.h>
#include <flatbuffers/idl.h>

namespace zeroeq
{
namespace detail
{

class FBEvent
{
public:
    FBEvent( const uint128_t& type_, const EventFunc& func_ )
        : type( type_ )
        , data( 0 )
        , size( 0 )
        , func( func_ )
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
            return data;
        return parser.builder_.GetBufferPointer();
    }

    void setData( const uint8_t* data_, const size_t size_ )
    {
        parser.builder_.Clear();
        data = data_;
        size = size_;
    }

    const uint128_t type;

    /** store the serialized data in here */
    flatbuffers::Parser parser;

    /** setData() uses this instead of fbb during deserialization */
    const uint8_t* data;
    size_t size;
    EventFunc func;

private:
    FBEvent( const FBEvent& ) = delete;
    FBEvent& operator=( const FBEvent& ) = delete;
};

}
}

#endif
