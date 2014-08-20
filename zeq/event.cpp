
/* Copyright (c) 2014, Human Brain Project
 *                     Daniel Nachbaur <daniel.nachbaur@epfl.ch>
 */

#include "event.h"
#include "detail/event.h"

namespace zeq
{

Event::Event( const uint64_t type )
    : _type( type )
    , _impl( new detail::Event )
{}

Event::Event( const Event& rhs )
    : _type( rhs._type )
    , _impl( new detail::Event( *rhs._impl))
    , _data( rhs._data )
{
}

Event::Event( Event& rhs )
    : _type( rhs._type )
    , _impl( rhs._impl )
    , _data( rhs._data )
{
    rhs._impl = 0;
    rhs._data.clear();
}

Event::~Event()
{
    delete _impl;
}

uint64_t Event::getType() const
{
    return _type;
}

size_t Event::getSize() const
{
    if( _impl )
        return _impl->getSize();
    return _data.size();
}

const void* Event::getData() const
{
    if( _impl )
        return _impl->getData();
    return _data.data();
}

void Event::setData( const void* data, const size_t size )
{
    delete _impl;
    _impl = 0;
    _data.resize( size );
    memcpy( _data.data(), data, size );
}

detail::Event* Event::getImpl()
{
    return _impl;
}

}
