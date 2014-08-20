
/* Copyright (c) 2014, Human Brain Project
 *                     Daniel Nachbaur <daniel.nachbaur@epfl.ch>
 */

#ifndef ZEQ_DETAIL_SERIALIZATION_H
#define ZEQ_DETAIL_SERIALIZATION_H

#include <zeq/camera_generated.h>
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

    flatbuffers::FlatBufferBuilder& fbb = event.getImpl()->fbb;
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

}
}

#endif
