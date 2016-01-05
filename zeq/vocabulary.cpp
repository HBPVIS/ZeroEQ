
/* Copyright (c) 2014-2016, Human Brain Project
 *                          Juan Hernando <jhernando@fi.upm.es>
 *                          Daniel Nachbaur <daniel.nachbaur@epfl.ch>
 */

#include "vocabulary.h"

#include "detail/vocabulary.h"
#include "event.h"
#include "eventDescriptor.h"

#ifdef ZEQ_USE_ZEROBUF
#  include "detail/byteswap.h"
#  include <zerobuf/Schema.h>
#  include <cstring>
#endif

namespace zeq
{
namespace vocabulary
{

Event serializeVocabulary( const EventDescriptors& vocabulary )
{
    return detail::serializeVocabulary( vocabulary );
}

EventDescriptors deserializeVocabulary( const Event& event )
{
    return detail::deserializeVocabulary( event );
}

Event serializeRequest( const uint128_t& eventType )
{
    return detail::serializeRequest( eventType );
}

uint128_t deserializeRequest( const Event& event )
{
    return detail::deserializeRequest( event );
}

Event serializeEcho( const std::string& message )
{
    return detail::serializeEcho( message );
}

std::string deserializeEcho( const Event& event )
{
    return detail::deserializeEcho( event );
}

Event serializeJSON( const uint128_t& type, const std::string& json )
{
    return detail::serializeJSON( type, json );
}

std::string deserializeJSON( const Event& event )
{
    return detail::deserializeJSON( event );
}

#ifdef ZEQ_USE_ZEROBUF
template< class T > void _serialize( uint8_t*& iter, T value )
{
#ifdef COMMON_BIGENDIAN
    detail::byteswap( value );
#endif
    *reinterpret_cast< T* >( iter ) = value;
    iter += sizeof( value );
}

template<> void _serialize( uint8_t*& iter, std::string value )
{
    _serialize( iter, uint64_t( value.size( )));
    ::memcpy( iter, value.c_str(), value.length( ));
    iter += value.length();
}

template< class T > T _deserialize( const char*& iter )
{
    T value = *reinterpret_cast< const T* >( iter );
    iter += sizeof( value );
#ifdef COMMON_BIGENDIAN
    detail::byteswap( value );
#endif
    return value;
}

template<> std::string _deserialize( const char*& iter )
{
    const size_t size = _deserialize< uint64_t >( iter );
    const std::string value( iter, size );
    iter += size;
    return value;
}


Event serializeSchemas( const zerobuf::Schemas& schemas )
{
#  ifdef COMMON_BIGENDIAN
#    error missing byteswap implementation
#  endif

    size_t size = 8; // #schemas
    for( const zerobuf::Schema& schema : schemas )
    {
        size += 32 + 8; // schema header + #fields
        for( const zerobuf::Schema::Field& field : schema.fields )
        {
            const std::string& name =
                std::get< zerobuf::Schema::FIELD_NAME >( field );
            size += 8 + name.length() + 40; // string length + string + fields
        }
    }

    uint8_t* iter = new uint8_t[size];
    ConstByteArray data( iter, std::default_delete< uint8_t[] >( ));

    _serialize( iter, uint64_t( schemas.size( ))); // #schemas

    for( const zerobuf::Schema& schema : schemas )
    {
        // schema header + #fields
        _serialize( iter, uint64_t( schema.staticSize ));
        _serialize( iter, uint64_t( schema.numDynamics ));
        _serialize( iter, schema.type );
        _serialize( iter, uint64_t( schema.fields.size( )));

        for( const zerobuf::Schema::Field& field : schema.fields )
        {
            const std::string& name =
                std::get< zerobuf::Schema::FIELD_NAME >( field );

            _serialize( iter, name );
            _serialize( iter, std::get< zerobuf::Schema::FIELD_TYPE >( field ));
            _serialize( iter, uint64_t(
                            std::get< zerobuf::Schema::FIELD_OFFSET >( field )));
            _serialize( iter, uint64_t(
                            std::get< zerobuf::Schema::FIELD_SIZE >( field )));
            _serialize( iter, uint64_t(
                           std::get< zerobuf::Schema::FIELD_ELEMENTS >( field )));
        }
    }

    Event event( zerobuf::Schema::ZEROBUF_TYPE( ));
    event.setData( data, size );
    return event;
}

zerobuf::Schemas deserializeSchemas( const Event& event )
{
#  ifdef COMMON_BIGENDIAN
#    error missing byteswap implementation
#  endif

    const char* const start = reinterpret_cast< const char* >( event.getData( ));
    const char* iter = start;
    const size_t size = event.getSize();

    // #schemas
    const size_t nSchemas = _deserialize< uint64_t >( iter );
    if( size_t( iter - start ) > size )
        throw std::runtime_error( "Schema deserialization out of data" );

    zerobuf::Schemas schemas;
    while( schemas.size() < nSchemas )
    {
        // schema header + #fields
        const size_t staticSize = _deserialize< uint64_t >( iter );
        const size_t numDynamics = _deserialize< uint64_t >( iter );
        const uint128_t type = _deserialize< uint128_t >( iter );
        const size_t nFields = _deserialize< uint64_t >( iter );
        if( size_t( iter - start ) > size )
            throw std::runtime_error( "Schema deserialization out of data" );

        std::vector< zerobuf::Schema::Field > fields;
        while( fields.size() < nFields )
        {
            const std::string name = _deserialize< std::string >( iter );
            const uint128_t fieldType = _deserialize< uint128_t >( iter );
            const size_t offset = _deserialize< uint64_t >( iter );
            const size_t fieldSize = _deserialize< uint64_t >( iter );
            const size_t nElements = _deserialize< uint64_t >( iter );
            if( size_t( iter - start ) > size )
                throw std::runtime_error( "Schema deserialization out of data" );

            fields.emplace_back( std::make_tuple( name, fieldType, offset,
                                                  fieldSize, nElements ));
        }

        schemas.emplace_back( zerobuf::Schema{ staticSize, numDynamics, type,
                                               std::move( fields )});
    }
    return schemas;
}

#else
Event serializeSchemas( const zerobuf::Schemas& )
{
    throw std::runtime_error( "Missing ZeroBuf support" );
}
zerobuf::Schemas deserializeSchemas( const Event& )
{
    throw std::runtime_error( "Missing ZeroBuf support" );
}
#endif

void registerEvent( const uint128_t& type, const std::string& schema )
{
    detail::registerEvent( type, schema );
}

}
}
