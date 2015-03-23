
/* Copyright (c) 2015, Human Brain Project
 *                     grigori.chevtchenko@epfl.ch
 */

#include "eventDescriptor.h"
#include "detail/eventDescriptor.h"

namespace zeq
{

EventDescriptor::EventDescriptor( const std::string& restName,
                                  const uint128_t& eventType,
                                  const std::string& schema )
    : _impl( new detail::EventDescriptor( restName, eventType, schema ))
{}

EventDescriptor::EventDescriptor( EventDescriptor&& rhs )
    : _impl( rhs._impl )
{
    rhs._impl = 0;
}

EventDescriptor::~EventDescriptor()
{
    delete _impl;
}

const std::string& EventDescriptor::getRestName() const
{
    return _impl->getRestName();
}

const uint128_t& EventDescriptor::getEventType() const
{
    return _impl->getEventType();
}

const std::string& EventDescriptor::getSchema() const
{
    return _impl->getSchema();
}

}
