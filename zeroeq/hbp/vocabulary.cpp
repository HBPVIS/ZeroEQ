/* Copyright (c) 2014-2015, Human Brain Project
 *                          Daniel Nachbaur <daniel.nachbaur@epfl.ch>
 *                          Juan Hernando <jhernando@fi.upm.es@epfl.ch>
 *                          Grigori Chevtchenko <grigori.chevtchenko@epfl.ch>
 */

#include "vocabulary.h"

#include <zeroeq/hbp/camera_generated.h>
#include <zeroeq/hbp/cellSetBinaryOp_generated.h>
#include <zeroeq/hbp/frame_generated.h>
#include <zeroeq/hbp/imageJPEG_generated.h>
#include <zeroeq/hbp/selections_generated.h>
#include <zeroeq/hbp/lookupTable1D_generated.h>
#include <zeroeq/event.h>
#include <zeroeq/vocabulary.h>

#include <cassert>

namespace zeroeq
{
namespace hbp
{

typedef std::vector< unsigned int > uints;

template< typename T, typename Builder >
void buildVectorOnlyBuffer(
    zeroeq::Event& event,
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
    const zeroeq::Event& event,
    const flatbuffers::Vector< T >* (U::*getter)( ) const )
{
    auto data = flatbuffers::GetRoot< U >( event.getData( ));
    return deserializeVector(( data->*getter )( ));
}

#define BUILD_VECTOR_ONLY_BUFFER( event, type, vector ) \
  buildVectorOnlyBuffer( event, &type##Builder::add_##vector, vector);

zeroeq::Event serializeCamera( const std::vector< float >& matrix )
{
    assert( matrix.size() == 16 );
    zeroeq::Event event( EVENT_CAMERA );
    BUILD_VECTOR_ONLY_BUFFER( event, Camera, matrix );
    return event;
}

std::vector< float > deserializeCamera( const Event& event )
{
    auto data = GetCamera( event.getData( ));
    assert( data->matrix()->Length() == 16 );
    return deserializeVector( data->matrix( ));
}

ZEROEQ_API Event serializeFrame( const data::Frame& frame )
{
    ::zeroeq::Event event( ::zeroeq::hbp::EVENT_FRAME );
    flatbuffers::FlatBufferBuilder& fbb = event.getFBB();

    FrameBuilder builder( fbb );
    builder.add_start( frame.start );
    const uint32_t current = frame.current > frame.start ?
                                 frame.current : frame.start;
    builder.add_current( current );
    builder.add_end( frame.end > current ? frame.end : current );
    builder.add_delta( frame.delta );

    fbb.Finish( builder.Finish( ));
    return event;
}

ZEROEQ_API data::Frame deserializeFrame( const Event& event )
{
    auto data = GetFrame( event.getData( ));
    return data::Frame( data->start(), data->current(), data->end(),
                        data->delta( ));
}

::zeroeq::Event serializeImageJPEG( const data::ImageJPEG& image )
{
    ::zeroeq::Event event( EVENT_IMAGEJPEG );
    flatbuffers::FlatBufferBuilder& fbb = event.getFBB();
    auto imageData = fbb.CreateVector( image.getDataPtr(),
                                       image.getSizeInBytes( ));

    ImageJPEGBuilder builder( fbb );
    builder.add_data( imageData );

    fbb.Finish( builder.Finish() );
    return event;
}

data::ImageJPEG deserializeImageJPEG( const ::zeroeq::Event& event )
{
    auto data = GetImageJPEG( event.getData( ) );
    return data::ImageJPEG( data->data()->size(), data->data()->Data( ));
}

Event serializeSelectedIDs( const uint32_ts& ids )
{
    zeroeq::Event event( EVENT_SELECTEDIDS );
    BUILD_VECTOR_ONLY_BUFFER( event, SelectedIDs, ids );
    return event;
}

uints deserializeSelectedIDs( const Event& event )
{
    return deserializeVector( event, &SelectedIDs::ids );
}

zeroeq::Event serializeToggleIDRequest( const uint32_ts& ids )
{
    zeroeq::Event event( EVENT_TOGGLEIDREQUEST );
    BUILD_VECTOR_ONLY_BUFFER( event, ToggleIDRequest, ids );
    return event;
}

uints deserializeToggleIDRequest( const zeroeq::Event& event )
{
    return deserializeVector( event, &ToggleIDRequest::ids );
}

zeroeq::Event serializeLookupTable1D( const std::vector< uint8_t >& lut )
{
    assert( lut.size() == 1024 );
    zeroeq::Event event( EVENT_LOOKUPTABLE1D );
    BUILD_VECTOR_ONLY_BUFFER( event, LookupTable1D, lut );
    return event;
}

std::vector< uint8_t > deserializeLookupTable1D( const Event& event )
{
    auto data = GetLookupTable1D( event.getData( ));
    assert( data->lut()->Length() == 1024 );
    return deserializeVector( data->lut( ));
}

Event serializeCellSetBinaryOp( const data::CellSetBinaryOp& cellSetBinaryOp )
{
  zeroeq::Event event( EVENT_CELLSETBINARYOP );

  flatbuffers::FlatBufferBuilder& fbb = event.getFBB( );

  auto firstData = fbb.CreateVector( cellSetBinaryOp.first );
  auto secondData = fbb.CreateVector( cellSetBinaryOp.second );

  CellSetBinaryOpBuilder builder( fbb );
  builder.add_first( firstData );
  builder.add_second( secondData );
  builder.add_operation( cellSetBinaryOp.operation );

  fbb.Finish( builder.Finish( ));

  return event;
}

Event serializeCellSetBinaryOp( const uint32_ts& first, const uint32_ts& second,
                                CellSetBinaryOpType type )
{
    return serializeCellSetBinaryOp(
        data::CellSetBinaryOp( first, second, type ));
}

data::CellSetBinaryOp
deserializeCellSetBinaryOp( const Event& event )
{
  data::CellSetBinaryOp result;

  auto data = GetCellSetBinaryOp( event.getData( ));

  return data::CellSetBinaryOp( deserializeVector( data->first( )),
                                deserializeVector( data->second( )),
                                CellSetBinaryOpType(data->operation( )));
}

}
}
