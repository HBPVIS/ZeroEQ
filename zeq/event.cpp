
/* Copyright (c) 2014-2015, Human Brain Project
 *                          Daniel Nachbaur <daniel.nachbaur@epfl.ch>
 */

#include "event.h"
#include "detail/event.h"

namespace zeq
{

Event::Event( const uint128_t& type )
    : _impl( new detail::Event( type ))
{}

Event::Event( Event&& rhs )
    : _impl( rhs._impl )
{
    rhs._impl = 0;
}

Event::~Event()
{
    delete _impl;
}

const uint128_t& Event::getType() const
{
    return _impl->type;
}

size_t Event::getSize() const
{
    return _impl->getSize();
}

const void* Event::getData() const
{
    return _impl->getData();
}

flatbuffers::FlatBufferBuilder& Event::getFBB()
{
    return _impl->parser.builder_;
}

flatbuffers::Parser& Event::getParser()
{
    return _impl->parser;
}

void Event::setData( const ConstByteArray& data, const size_t size )
{
    _impl->setData( data, size );
}

}
