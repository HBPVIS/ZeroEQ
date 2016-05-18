
/* Copyright (c) 2014-2015, Human Brain Project
 *                          Daniel Nachbaur <daniel.nachbaur@epfl.ch>
 */

#include "fbevent.h"
#include "detail/fbevent.h"

namespace zeroeq
{

FBEvent::FBEvent( const uint128_t& type, const EventFunc& func )
    : _impl( new detail::FBEvent( type, func ))
{
    if( func )
        registerDeserializedCallback( [this](){ _impl->func( *this ); } );
}

FBEvent::~FBEvent()
{
}

std::string FBEvent::getTypeName() const
{
    ZEROEQTHROW( std::runtime_error(
                     "Flatbuffers objects do not have a type name" ));
}

uint128_t FBEvent::getTypeIdentifier() const
{
    return _impl->type;
}

size_t FBEvent::getSize() const
{
    return _impl->getSize();
}

const void* FBEvent::getData() const
{
    return _impl->getData();
}

flatbuffers::FlatBufferBuilder& FBEvent::getFBB()
{
    return _impl->parser.builder_;
}

flatbuffers::Parser& FBEvent::getParser()
{
    return _impl->parser;
}

FBEvent::FBEvent( FBEvent&& rhs )
    : _impl( std::move( rhs._impl ))
{
}

bool FBEvent::_fromBinary( const void *data, const size_t size )
{
    _impl->setData(( const uint8_t*)data, size );
    return true;
}

servus::Serializable::Data FBEvent::_toBinary() const
{
    servus::Serializable::Data data;
    data.ptr.reset( _impl->getData(), []( const void* ){});
    data.size = _impl->getSize();
    return data;
}

bool FBEvent::_fromJSON( const std::string& )
{
    ZEROEQTHROW( std::runtime_error(
                     "Flatbuffers objects cannot be converted from JSON" ));
}

std::string FBEvent::_toJSON() const
{
    ZEROEQTHROW( std::runtime_error(
                     "Flatbuffers objects cannot be converted to JSON" ));
}

}
