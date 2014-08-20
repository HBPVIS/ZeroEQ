
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

class Event
{
public:
    Event()
    {}

    size_t getSize() const
    {
        return fbb.GetSize();
    }

    const void* getData() const
    {
        return fbb.GetBufferPointer();
    }

    flatbuffers::FlatBufferBuilder fbb;
};

}
}

#endif
