
/* Copyright (c) 2014-2015, Human Brain Project
 *                          Daniel Nachbaur <daniel.nachbaur@epfl.ch>
 */

#ifndef ZEQ_DETAIL_EVENT_H
#define ZEQ_DETAIL_EVENT_H

#include "vocabulary.h"

#include <zeq/types.h>
#include <zeq/event.h>

#include <lunchbox/buffer.h>
#include <flatbuffers/flatbuffers.h>
#include <flatbuffers/idl.h>

namespace zeq
{
namespace detail
{

class Event : public boost::noncopyable
{
public:
    Event( const uint128_t& type_ )
        : type( type_ )
    {
        const std::string& schema = vocabulary::detail::getSchema( type );

        // populate parser with schema from type
        if( !schema.empty() && !parser.Parse( schema.c_str( )))
            throw std::runtime_error( parser.error_ );
    }

    size_t getSize() const
    {
        if( !data.isEmpty( ))
            return data.getSize();
        return parser.builder_.GetSize();
    }

    const void* getData() const
    {
        if( !data.isEmpty( ))
            return data.getData();
        return parser.builder_.GetBufferPointer();
    }

    void setData( const void* data_, const size_t size )
    {
        parser.builder_.Clear();
        data.replace( data_, size );
    }

    const uint128_t type;

    /** store the serialized data in here */
    flatbuffers::Parser parser;

    /** setData() uses this instead of fbb during deserialization */
    lunchbox::Bufferb data;
};

}
}

#endif
