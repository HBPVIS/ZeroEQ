/* Copyright (c) 2014, Human Brain Project
 *                     Daniel Nachbaur <daniel.nachbaur@epfl.ch>
 *                     Juan Hernando <jhernando@fi.upm.es@epfl.ch>
 */

#include "vocabulary.h"

#include "zeqBBP/camera_generated.h"
#include "zeqBBP/selection_generated.h"

#include "zeq/event.h"
#include "zeq/vocabulary.h"

#include <lunchbox/debug.h>

namespace zeq
{
namespace bbp
{

Event serializeCamera( const std::vector< float >& matrix )
{
    LBASSERT( matrix.size() == 16 )
    zeq::Event event( EVENT_CAMERA );

    flatbuffers::FlatBufferBuilder& fbb = event.getFBB();
    CameraBuilder builder( fbb );
    builder.add_matrix( fbb.CreateVector( matrix.data(), matrix.size( )));
    fbb.Finish( builder.Finish( ));
    return event;
}

std::vector< float > deserializeCamera( const Event& camera )
{
    auto data = GetCamera( camera.getData( ));
    LBASSERT( data->matrix()->Length() == 16 );

    std::vector< float > returnMatrix( data->matrix()->Length( ));
    for( flatbuffers::uoffset_t i = 0; i < data->matrix()->Length(); ++i )
        returnMatrix[i] = data->matrix()->Get(i);
    return returnMatrix;
}


Event serializeSelection( const std::vector< unsigned int >& selection )
{
    zeq::Event event( EVENT_SELECTION );

    flatbuffers::FlatBufferBuilder& fbb = event.getFBB();
    SelectionBuilder builder( fbb );
    builder.add_ids( fbb.CreateVector( selection.data(), selection.size() ));
    fbb.Finish( builder.Finish( ));
    return event;
}

std::vector< unsigned int > deserializeSelection( const Event& selection )
{
    auto data = GetSelection( selection.getData( ));

    std::vector< unsigned int > returnSelection( data->ids()->Length( ));
    for( flatbuffers::uoffset_t i = 0; i < data->ids()->Length(); ++i )
        returnSelection[i] = data->ids()->Get(i);
    return returnSelection;
}


}
}
