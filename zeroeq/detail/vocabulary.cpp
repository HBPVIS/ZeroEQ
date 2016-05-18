
/* Copyright (c) 2014-2015, Human Brain Project
 *                          Daniel Nachbaur <daniel.nachbaur@epfl.ch>
 */

#include "fbevent.h"

#include "../vocabulary.h"

#include <zeroeq/echo_generated.h>

#include <unordered_map>
#include <stdexcept>

namespace zeroeq
{
namespace vocabulary
{
namespace detail
{

zeroeq::FBEvent serializeEcho( const std::string& msg )
{
    zeroeq::FBEvent event( ::zeroeq::vocabulary::EVENT_ECHO,
                           ::zeroeq::EventFunc( ));

    flatbuffers::FlatBufferBuilder& fbb = event.getFBB();
    EchoBuilder builder( fbb );
    builder.add_message( fbb.CreateString( msg ));
    fbb.Finish( builder.Finish( ));
    return event;
}

std::string deserializeEcho( const zeroeq::FBEvent& event )
{
    const auto& data = GetEcho( event.getData( ));
    return data->message()->c_str();
}

}
}
}
