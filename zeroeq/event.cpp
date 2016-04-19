
/* Copyright (c) 2014-2015, Human Brain Project
 *                          Daniel Nachbaur <daniel.nachbaur@epfl.ch>
 */

#include "event.h"
#include "detail/event.h"

namespace zeroeq
{

Event::Event( const uint128_t& type, const EventFunc& func )
    : _impl( new detail::Event( type, func ))
{
    setUpdatedFunction( [this](){ _impl->func( *this ); });
}

Event::~Event()
{
}

std::string Event::getTypeName() const
{
    return _impl->type.getString();
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

Event::Event( Event&& rhs )
    : _impl( std::move( rhs._impl ))
{
}

bool Event::_fromBinary( const void *data, const size_t size )
{
    _impl->setData(( const uint8_t*)data, size );
    return true;
}

servus::Serializable::Data Event::_toBinary() const
{
    servus::Serializable::Data data;
    data.ptr.reset( _impl->getData(), []( const void* ){});
    data.size = _impl->getSize();
    return data;
}

bool Event::_fromJSON( const std::string& )
{
    ZEROEQTHROW( std::runtime_error(
                     "Flatbuffers objects cannot be converted from JSON" ));
}

std::string Event::_toJSON() const
{
    ZEROEQTHROW( std::runtime_error(
                     "Flatbuffers objects cannot be converted to JSON" ));
}

}
