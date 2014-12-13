
/* Copyright (c) 2014, Human Brain Project
 *                     Daniel Nachbaur <daniel.nachbaur@epfl.ch>
 */

#ifndef ZEQ_DETAIL_SERIALIZATION_H
#define ZEQ_DETAIL_SERIALIZATION_H

#include <zeq/echo_generated.h>
#include <zeq/vocabulary.h>
#include "event.h"
#include <lunchbox/debug.h>

namespace zeq
{
namespace detail
{

zeq::Event serializeEcho( const std::string& msg )
{
    zeq::Event event( vocabulary::EVENT_ECHO );

    flatbuffers::FlatBufferBuilder& fbb = event.getFBB();
    EchoBuilder builder( fbb );
    builder.add_message( fbb.CreateString ( msg ));
    fbb.Finish( builder.Finish( ));
    return event;
}

std::string deserializeEcho( const zeq::Event& event )
{
    auto data = GetEcho( event.getData( ));
    return data->message()->c_str();
}

}
}
#endif
