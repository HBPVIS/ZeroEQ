
/* Copyright (c) 2014, Human Brain Project
 *                     Daniel Nachbaur <daniel.nachbaur@epfl.ch>
 */

#ifndef ZEQ_DETAIL_EVENT_H
#define ZEQ_DETAIL_EVENT_H

#include <zeq/types.h>
#include <zeq/event.h>
#include <zeq/log.h>

#include <flatbuffers/flatbuffers.h>
#include <flatbuffers/idl.h>

namespace zeq
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
    Event( const uint128_t& type_ )
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
