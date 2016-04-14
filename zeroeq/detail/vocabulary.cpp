
/* Copyright (c) 2014-2015, Human Brain Project
 *                          Daniel Nachbaur <daniel.nachbaur@epfl.ch>
 */

#include "event.h"

#include "../eventDescriptor.h"
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

namespace
{
typedef std::unordered_map< uint128_t, std::string > EventRegistry;

EventRegistry& getRegistry()
{
    static EventRegistry _eventRegistry;
    return _eventRegistry;
}

const std::string& getSchema( const uint128_t& type )
{
    static const std::string nullString;
    const EventRegistry& registry = getRegistry();
    EventRegistry::const_iterator i = registry.find( type );
    return ( i == registry.end( )) ? nullString : i->second;
}
}

Event serializeVocabulary( const EventDescriptors& vocabulary )
{
    ::zeroeq::Event event( EVENT_VOCABULARY );
    flatbuffers::FlatBufferBuilder& fbb = event.getFBB();

    std::vector< flatbuffers::Offset<flatbuffers::String > > restNames;
    std::vector< uint64_t > eventTypeLows;
    std::vector< uint64_t > eventTypeHighs;
    std::vector< flatbuffers::Offset<flatbuffers::String > > schemas;
    std::vector< uint8_t > eventDirections;

    for( const auto& eventDescriptor : vocabulary )
    {
        restNames.push_back( fbb.CreateString( eventDescriptor.getRestName( )));
        eventTypeHighs.push_back( eventDescriptor.getEventType().high( ));
        eventTypeLows.push_back( eventDescriptor.getEventType().low( ));
        schemas.push_back( fbb.CreateString( eventDescriptor.getSchema( )));
        eventDirections.push_back(
                                uint8_t( eventDescriptor.getEventDirection( )));
    }

    const auto& restNamesForZeroEQ = fbb.CreateVector( restNames );
    const auto& eventTypeHighsForZeroEQ = fbb.CreateVector( eventTypeHighs );
    const auto& eventTypeLowsForZeroEQ = fbb.CreateVector( eventTypeLows );
    const auto& schemasForZeroEQ = fbb.CreateVector( schemas );
    const auto& eventDirectionsForZeroEQ = fbb.CreateVector( eventDirections );

    VocabularyBuilder builder( fbb );
    builder.add_restNames( restNamesForZeroEQ );
    builder.add_eventHighs( eventTypeHighsForZeroEQ );
    builder.add_eventLows( eventTypeLowsForZeroEQ );
    builder.add_schemas( schemasForZeroEQ );
    builder.add_eventDirections( eventDirectionsForZeroEQ );

    fbb.Finish( builder.Finish( ));
    return event;
}

EventDescriptors deserializeVocabulary( const Event& event )
{
    const auto& data = GetVocabulary( event.getData( ));

    EventDescriptors vocabulary;
    vocabulary.reserve( data->restNames()->Length( ));
    for( flatbuffers::uoffset_t i = 0; i < data->restNames()->Length(); ++i )
    {
        const uint128_t eventType( data->eventHighs()->Get(i),
                                   data->eventLows()->Get(i));
        EventDescriptor restZeroEQEvent( data->restNames()->Get(i)->c_str(),
                                         eventType,
                                         data->schemas()->Get(i)->c_str(),
                                         zeroeq::EventDirection(
                                             data->eventDirections()->Get(i)));
        vocabulary.push_back( std::move( restZeroEQEvent ));
    }

    return vocabulary;
}

::zeroeq::Event serializeRequest( const uint128_t& eventType )
{
    ::zeroeq::Event event( ::zeroeq::vocabulary::EVENT_REQUEST );
    flatbuffers::FlatBufferBuilder& fbb = event.getFBB();

    RequestBuilder builder( fbb );
    builder.add_eventHigh( eventType.high( ));
    builder.add_eventLow( eventType.low( ));

    fbb.Finish( builder.Finish( ));
    return event;
}

uint128_t deserializeRequest( const ::zeroeq::Event& event )
{
    const auto& data = GetRequest( event.getData( ));
    return uint128_t( data->eventHigh(), data->eventLow( ));
}

zeroeq::Event serializeEcho( const std::string& msg )
{
    zeroeq::Event event( ::zeroeq::vocabulary::EVENT_ECHO );

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

zeroeq::Event serializeJSON( const uint128_t& type, const std::string& json )
{
    const std::string& schema = getSchema( type );
    if( schema.empty( ))
        ZEROEQTHROW( std::runtime_error(
                         "JSON schema for event not registered" ));

    zeroeq::Event event( type );
    flatbuffers::Parser& parser = event.getParser();
    if( !parser.Parse( schema.c_str( )) || !parser.Parse( json.c_str( )))
        ZEROEQTHROW( std::runtime_error( parser.error_ ));
    return event;
}

std::string deserializeJSON( const zeroeq::Event& event )
{
    const std::string& schema = getSchema( event.getType( ));
    if( schema.empty( ))
        ZEROEQTHROW( std::runtime_error(
                         "JSON schema for event not registered" ));

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

void registerEvent( const uint128_t& type, const std::string& schema )
{
    getRegistry()[type] = schema;
}

}
}
}
