/* Copyright (c) 2014, Human Brain Project
 *                     Daniel Nachbaur <daniel.nachbaur@epfl.ch>
 *                     Juan Hernando <jhernando@fi.upm.es@epfl.ch>
 */

#include "vocabulary.h"

#include "zeqHBP/camera_generated.h"
#include "zeqHBP/selections_generated.h"

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

Event serializeSelectedIDs( const uints& ids )
{
    zeq::Event event( EVENT_SELECTED_IDS );
    BUILD_VECTOR_ONLY_BUFFER( event, SelectedIDs, ids, ids );
    return event;
}

uints deserializeSelectedIDs( const Event& event )
{
    return deserializeVector( event, &SelectedIDs::ids );
}

zeq::Event serializeToggleIDRequest( const uints& ids )
{
    zeq::Event event( EVENT_TOGGLE_ID_REQUEST );
    BUILD_VECTOR_ONLY_BUFFER( event, ToggleIDRequest, ids, ids );
    return event;
}

uints deserializeToggleIDRequest( const zeq::Event& event )
{
    return deserializeVector( event, &ToggleIDRequest::ids );
}


#undef BUILD_VECTOR_ONLY_BUFFER

}
}
