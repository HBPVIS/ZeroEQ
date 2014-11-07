
/* Copyright (c) 2014, Human Brain Project
 *                     Daniel Nachbaur <daniel.nachbaur@epfl.ch>
 */

#ifndef ZEQ_DETAIL_SERIALIZATION_H
#define ZEQ_DETAIL_SERIALIZATION_H

#include <zeq/camera_generated.h>
#include <zeq/selection_generated.h>
#include <zeq/vocabulary.h>
#include "event.h"
#include <lunchbox/debug.h>

namespace zeq
{
namespace detail
{

zeq::Event serializeCamera( const std::vector< float >& matrix )
{
    if( matrix.size() != 16 )
        return zeq::Event( vocabulary::EVENT_INVALID );

    zeq::Event event( vocabulary::EVENT_CAMERA );

    flatbuffers::FlatBufferBuilder& fbb = event.getFBB();
    CameraBuilder builder( fbb );
    builder.add_matrix( fbb.CreateVector( matrix.data(), 16 ));
    fbb.Finish( builder.Finish( ));
    return event;
}

std::vector< float > deserializeCamera( const zeq::Event& camera )
{
    auto data = GetCamera( camera.getData( ));
    LBASSERT( data->matrix()->Length() == 16 );

    std::vector< float > returnMatrix( data->matrix()->Length( ));
    for( size_t i = 0; i < data->matrix()->Length(); ++i )
        returnMatrix[i] = data->matrix()->Get(i);
    return returnMatrix;
}


zeq::Event serializeSelection( const std::vector< unsigned int >& selection )
{
    zeq::Event event( vocabulary::EVENT_SELECTION );

    flatbuffers::FlatBufferBuilder& fbb = event.getFBB();
    SelectionBuilder builder( fbb );
    builder.add_ids( fbb.CreateVector( selection.data(), selection.size() ));
    fbb.Finish( builder.Finish( ));
    return event;
}

std::vector< unsigned int > deserializeSelection( const zeq::Event& selection )
{
    auto data = GetSelection( selection.getData( ));

    std::vector< unsigned int > returnSelection( data->ids()->Length( ));
    for( size_t i = 0; i < data->ids()->Length(); ++i )
        returnSelection[i] = data->ids()->Get(i);
    return returnSelection;
}


}
}

#endif
