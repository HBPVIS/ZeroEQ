/* Copyright (c) 2014-2015, Human Brain Project
 *                          Daniel Nachbaur <daniel.nachbaur@epfl.ch>
 *                          Juan Hernando <jhernando@fi.upm.es@epfl.ch>
 *                          Grigori Chevtchenko <grigori.chevtchenko@epfl.ch>
 */

#include "vocabulary.h"

#include "zeq/hbp/camera_generated.h"
#include "zeq/hbp/imageJPEG_generated.h"
#include "zeq/hbp/lookupTable1D_generated.h"
#include "zeq/hbp/request_generated.h"
#include "zeq/hbp/selections_generated.h"
#include "zeq/event.h"
#include "zeq/vocabulary.h"

#include <lunchbox/debug.h>

namespace zeq
{
namespace hbp
{

typedef std::vector< unsigned int > uints;

template< typename T, typename Builder >
void buildVectorOnlyBuffer(
    zeq::Event& event,
    void (Builder::*adder)( flatbuffers::Offset< flatbuffers::Vector< T >>),
    const std::vector< T >& vector)
{
    flatbuffers::FlatBufferBuilder& fbb = event.getFBB();
    Builder builder( fbb );
    (builder.*adder)( fbb.CreateVector( vector.data(), vector.size() ));
    fbb.Finish( builder.Finish( ));
}

template< typename T >
std::vector< T > deserializeVector( const flatbuffers::Vector< T >* in )
{
    std::vector< T > out( in->Length( ));
    for( flatbuffers::uoffset_t i = 0; i < in->Length(); ++i )
        out[i] = in->Get( i );
    return out;
}

template< typename T, typename U >
std::vector< T > deserializeVector(
    const zeq::Event& event,
    const flatbuffers::Vector< T >* (U::*getter)( ) const )
{
    auto data = flatbuffers::GetRoot< U >( event.getData( ));
    return deserializeVector(( data->*getter )( ));
}

#define BUILD_VECTOR_ONLY_BUFFER( event, type, attribute, vector ) \
  buildVectorOnlyBuffer( event, &type##Builder::add_##attribute, vector);

zeq::Event serializeCamera( const std::vector< float >& matrix )
{
    LBASSERT( matrix.size() == 16 )
    zeq::Event event( EVENT_CAMERA );
    BUILD_VECTOR_ONLY_BUFFER( event, Camera, matrix, matrix );
    return event;
}

std::vector< float > deserializeCamera( const Event& event )
{
    auto data = GetCamera( event.getData( ));
    LBASSERT( data->matrix()->Length() == 16 );
    return deserializeVector( data->matrix( ));
}

::zeq::Event serializeImageJPEG( const data::ImageJPEG& image )
{
    ::zeq::Event event( EVENT_IMAGEJPEG );
    flatbuffers::FlatBufferBuilder& fbb = event.getFBB();
    auto imageData = fbb.CreateVector( image.getDataPtr(), image.getSizeInBytes() );

    ImageJPEGBuilder builder( fbb );
    builder.add_data( imageData );

    fbb.Finish( builder.Finish() );
    return event;
}

data::ImageJPEG deserializeImageJPEG( const ::zeq::Event& event )
{
    auto data = GetImageJPEG( event.getData( ) );
    data::ImageJPEG result( data->data()->size(), data->data()->Data() );

    return result;
}

::zeq::Event serializeRequest( const lunchbox::uint128_t& eventType)
{
    ::zeq::Event event( EVENT_REQUEST );
    flatbuffers::FlatBufferBuilder& fbb = event.getFBB();

    RequestBuilder builder( fbb );
    builder.add_eventLow( eventType.low());
    builder.add_eventHigh( eventType.high());

    fbb.Finish( builder.Finish( ));
    return event;
}

lunchbox::uint128_t deserializeRequest( const ::zeq::Event& event )
{
    auto data = GetRequest( event.getData( ));
    return lunchbox::uint128_t( data->eventHigh(), data->eventLow());
}

Event serializeSelectedIDs( const uints& ids )
{
    zeq::Event event( EVENT_SELECTEDIDS );
    BUILD_VECTOR_ONLY_BUFFER( event, SelectedIDs, ids, ids );
    return event;
}

uints deserializeSelectedIDs( const Event& event )
{
    return deserializeVector( event, &SelectedIDs::ids );
}

zeq::Event serializeToggleIDRequest( const uints& ids )
{
    zeq::Event event( EVENT_TOGGLEIDREQUEST );
    BUILD_VECTOR_ONLY_BUFFER( event, ToggleIDRequest, ids, ids );
    return event;
}

uints deserializeToggleIDRequest( const zeq::Event& event )
{
    return deserializeVector( event, &ToggleIDRequest::ids );
}

zeq::Event serializeLookupTable1D( const std::vector< uint8_t >& lut )
{
    LBASSERT( lut.size() == 1024 )
    zeq::Event event( EVENT_LOOKUPTABLE1D );
    BUILD_VECTOR_ONLY_BUFFER( event, LookupTable1D, lut, lut );
    return event;
}

std::vector< uint8_t > deserializeLookupTable1D( const Event& event )
{
    auto data = GetLookupTable1D( event.getData( ));
    LBASSERT( data->lut()->Length() == 1024 );
    return deserializeVector( data->lut( ));
}

}
}
