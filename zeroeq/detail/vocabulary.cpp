
/* Copyright (c) 2014-2015, Human Brain Project
 *                          Daniel Nachbaur <daniel.nachbaur@epfl.ch>
 */

#include "event.h"

#include "../vocabulary.h"

#include <zeroeq/echo_generated.h>
#include <zeroeq/request_generated.h>
#include <zeroeq/vocabulary_generated.h>

#include <flatbuffers/idl.h>
#include <unordered_map>
#include <stdexcept>

namespace zeroeq
{
namespace vocabulary
{
namespace detail
{

zeroeq::Event serializeEcho( const std::string& msg )
{
    zeroeq::Event event( ::zeroeq::vocabulary::EVENT_ECHO, ::zeroeq::EventFunc( ));

    flatbuffers::FlatBufferBuilder& fbb = event.getFBB();
    EchoBuilder builder( fbb );
    builder.add_message( fbb.CreateString( msg ));
    fbb.Finish( builder.Finish( ));
    return event;
}

std::string deserializeEcho( const zeroeq::Event& event )
{
    const auto& data = GetEcho( event.getData( ));
    return data->message()->c_str();
}

}
}
}
