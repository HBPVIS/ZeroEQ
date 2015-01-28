
/* Copyright (c) 2014-2015, Human Brain Project
 *                          Daniel Nachbaur <daniel.nachbaur@epfl.ch>
 */

#include "event.h"

#include <zeq/echo_generated.h>
#include <zeq/vocabulary.h>

#include <lunchbox/debug.h>
#include <lunchbox/stdExt.h>

namespace zeq
{
namespace vocabulary
{
namespace detail
{
namespace
{
stde::hash_map< uint128_t, std::string >& getRegistry()
{
    static stde::hash_map< uint128_t, std::string > _eventRegistry;
    return _eventRegistry;
}
}

zeq::Event serializeEcho( const std::string& msg )
{
    zeq::Event event( ::zeq::vocabulary::EVENT_ECHO );

    flatbuffers::FlatBufferBuilder& fbb = event.getFBB();
    EchoBuilder builder( fbb );
    builder.add_message( fbb.CreateString( msg ));
    fbb.Finish( builder.Finish( ));
    return event;
}

std::string deserializeEcho( const zeq::Event& event )
{
    auto data = GetEcho( event.getData( ));
    return data->message()->c_str();
}

zeq::Event serializeJSON( const uint128_t& type, const std::string& json )
{
    zeq::Event event( type );
    flatbuffers::Parser& parser = event.getParser();
    if( !parser.Parse( json.c_str( )))
        throw std::runtime_error( parser.error_ );
    return event;
}

std::string deserializeJSON( const zeq::Event& event )
{
    std::string json;
    flatbuffers::GeneratorOptions opts;
    opts.base64_byte_array = true;
    GenerateText( event.getParser(), event.getData(), opts, &json );
    return json;
}

void registerEvent( const uint128_t& type, const std::string& schema )
{
    getRegistry()[type] = schema;
}

const std::string& getSchema( const uint128_t& type )
{
    return getRegistry()[type];
}

}
}
}
