/* Copyright (c) 2014-2015, Human Brain Project
 *                          Daniel Nachbaur <daniel.nachbaur@epfl.ch>
 *                          Juan Hernando <jhernando@fi.upm.es@epfl.ch>
 *                          Grigori Chevtchenko <grigori.chevtchenko@epfl.ch>
 */

#include "vocabulary.h"

#include <zeroeq/hbp/camera_generated.h>
#include <zeroeq/hbp/cellSetBinaryOp_generated.h>
#include <zeroeq/hbp/selections_generated.h>
#include <zeroeq/fbevent.h>
#include <zeroeq/vocabulary.h>

#include <cassert>

namespace zeroeq
{
namespace hbp
{

typedef std::vector< unsigned int > uints;

template< typename T, typename Builder >
void buildVectorOnlyBuffer(
    zeroeq::FBEvent& event,
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
    const zeroeq::FBEvent& event,
    const flatbuffers::Vector< T >* (U::*getter)( ) const )
{
    auto data = flatbuffers::GetRoot< U >( event.getData( ));
    return deserializeVector(( data->*getter )( ));
}

#define BUILD_VECTOR_ONLY_BUFFER( event, type, vector ) \
  buildVectorOnlyBuffer( event, &type##Builder::add_##vector, vector);

FBEvent serializeSelectedIDs( const uint32_ts& ids )
{
    FBEvent event( EVENT_SELECTEDIDS, zeroeq::EventFunc( ));
    BUILD_VECTOR_ONLY_BUFFER( event, SelectedIDs, ids );
    return event;
}

uints deserializeSelectedIDs( const FBEvent& event )
{
    return deserializeVector( event, &SelectedIDs::ids );
}

FBEvent serializeToggleIDRequest( const uint32_ts& ids )
{
    FBEvent event( EVENT_TOGGLEIDREQUEST, zeroeq::EventFunc( ));
    BUILD_VECTOR_ONLY_BUFFER( event, ToggleIDRequest, ids );
    return event;
}

uints deserializeToggleIDRequest( const zeroeq::FBEvent& event )
{
    return deserializeVector( event, &ToggleIDRequest::ids );
}

FBEvent serializeCellSetBinaryOp(
        const data::CellSetBinaryOp& cellSetBinaryOp )
{
  FBEvent event( EVENT_CELLSETBINARYOP, zeroeq::EventFunc( ));

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

FBEvent serializeCellSetBinaryOp( const uint32_ts& first, const uint32_ts& second,
                                  CellSetBinaryOpType type )
{
    return serializeCellSetBinaryOp(
        data::CellSetBinaryOp( first, second, type ));
}

data::CellSetBinaryOp deserializeCellSetBinaryOp( const FBEvent& event )
{
  data::CellSetBinaryOp result;

  auto data = GetCellSetBinaryOp( event.getData( ));

  return data::CellSetBinaryOp( deserializeVector( data->first( )),
                                deserializeVector( data->second( )),
                                CellSetBinaryOpType(data->operation( )));
}

}
}
