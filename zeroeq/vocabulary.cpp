
/* Copyright (c) 2014-2016, Human Brain Project
 *                          Juan Hernando <jhernando@fi.upm.es>
 *                          Daniel Nachbaur <daniel.nachbaur@epfl.ch>
 */

#include "vocabulary.h"
#include "log.h"
#include "detail/vocabulary.h"
#include "fbevent.h"

#include <flatbuffers/flatbuffers.h>
#include <flatbuffers/idl.h>

namespace zeroeq
{
namespace vocabulary
{

FBEvent serializeEcho( const std::string& message )
{
    return detail::serializeEcho( message );
}

std::string deserializeEcho( const FBEvent& event )
{
    return detail::deserializeEcho( event );
}

FBEvent serializeJSON( const std::string& json,
                       const uint128_t& type,
                       const std::string& schema )
{
    if( schema.empty( ))
        ZEROEQTHROW( std::runtime_error(
                       "JSON schema for event not registered" ));

    FBEvent event( type, EventFunc( ));
    flatbuffers::Parser& parser = event.getParser();
    if( !parser.Parse( schema.c_str( )) || !parser.Parse( json.c_str( )))
        ZEROEQTHROW( std::runtime_error( parser.error_ ));
    return event;
}

std::string deserializeJSON( const FBEvent& event,
                             const std::string& schema )
{
     flatbuffers::Parser parser;
     if( !parser.Parse( schema.c_str( )))
        ZEROEQTHROW( std::runtime_error( parser.error_ ));

     std::string json;
     flatbuffers::GeneratorOptions opts;
     opts.base64_byte_array = true;
     opts.strict_json = true;
     GenerateText( parser, event.getData(), opts, &json );
     return json;
}

}
}
