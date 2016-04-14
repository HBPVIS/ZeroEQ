
/* Copyright (c) 2014-2016, Human Brain Project
 *                          Juan Hernando <jhernando@fi.upm.es>
 *                          Daniel Nachbaur <danielnachbaur@epfl.ch>
 */

#define BOOST_TEST_MODULE zeroeq_serialization

#include <zeroeq/zeroeq.h>

#include <zeroeq/vocabulary_generated.h>
#include <zeroeq/vocabulary_zeroeq_generated.h>
#include <zeroeq/eventDescriptor.h>

#include <tests/binary_generated.h>
#include <tests/binary_zeroeq_generated.h>

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(vocabulary_serialization)
{
    zeroeq::EventDescriptors vocabulary;

    zeroeq::EventDescriptor onlySubscribedEvent(
                "testName01", zeroeq::uint128_t( 1 ),
                "testSchema01", zeroeq::SUBSCRIBER );
    zeroeq::EventDescriptor onlyPublishedEvent(
                "testName02", zeroeq::uint128_t( 2 ),
                "testSchema02", zeroeq::PUBLISHER );
    zeroeq::EventDescriptor bidirectionalEvent(
                "testName03",  zeroeq::uint128_t( 3 ),
                "testSchema03", zeroeq::BIDIRECTIONAL );
    vocabulary.push_back( std::move( onlySubscribedEvent ));
    vocabulary.push_back( std::move( onlyPublishedEvent ));
    vocabulary.push_back( std::move( bidirectionalEvent ));

    const zeroeq::Event& event =
            zeroeq::vocabulary::serializeVocabulary( vocabulary );

    zeroeq::EventDescriptors deserialized_vocabulary =
            zeroeq::vocabulary::deserializeVocabulary( event );

    BOOST_CHECK_EQUAL( deserialized_vocabulary[0].getRestName(), "testName01" );
    BOOST_CHECK_EQUAL( deserialized_vocabulary[0].getEventType(),
                       zeroeq::uint128_t( 1 ));
    BOOST_CHECK_EQUAL( deserialized_vocabulary[0].getSchema(), "testSchema01" );
    BOOST_CHECK_EQUAL( deserialized_vocabulary[0].getEventDirection(),
                       zeroeq::SUBSCRIBER );

    BOOST_CHECK_EQUAL( deserialized_vocabulary[1].getRestName(), "testName02" );
    BOOST_CHECK_EQUAL( deserialized_vocabulary[1].getEventType(),
                       zeroeq::uint128_t( 2 ));
    BOOST_CHECK_EQUAL( deserialized_vocabulary[1].getSchema(), "testSchema02" );
    BOOST_CHECK_EQUAL( deserialized_vocabulary[1].getEventDirection(),
                       zeroeq::PUBLISHER );

    BOOST_CHECK_EQUAL( deserialized_vocabulary[2].getRestName(), "testName03" );
    BOOST_CHECK_EQUAL( deserialized_vocabulary[2].getEventType(),
                       zeroeq::uint128_t( 3 ));
    BOOST_CHECK_EQUAL( deserialized_vocabulary[2].getSchema(), "testSchema03" );
    BOOST_CHECK_EQUAL( deserialized_vocabulary[2].getEventDirection(),
                       zeroeq::BIDIRECTIONAL );
}

BOOST_AUTO_TEST_CASE(request_serialization)
{
    const zeroeq::uint128_t eventType( zeroeq::vocabulary::EVENT_ECHO );
    const zeroeq::Event& requestEvent =
            zeroeq::vocabulary::serializeRequest( eventType );
    const zeroeq::uint128_t deserializedEventType =
            zeroeq::vocabulary::deserializeRequest( requestEvent );
    BOOST_CHECK_EQUAL( eventType, deserializedEventType );
}

BOOST_AUTO_TEST_CASE(event_registration)
{
    const std::string schema = "table Test { message:string; } root_type Test;";
    const zeroeq::uint128_t eventType( 42 );
    zeroeq::vocabulary::registerEvent( eventType, schema );

    const std::string json( "{\n"
                            "  \"message\": \"test message\"\n"
                            "}\n" );
    const zeroeq::Event& event =
            zeroeq::vocabulary::serializeJSON( eventType, json );
    const std::string& deserialized =
            zeroeq::vocabulary::deserializeJSON( event );
    BOOST_CHECK_EQUAL( json, deserialized );
}

BOOST_AUTO_TEST_CASE(invalid_event_registration)
{
    const std::string schema = "wrong Test { message:string; } root_type Test;";
    const zeroeq::uint128_t eventType( 42 );
    zeroeq::vocabulary::registerEvent( eventType, schema );

    zeroeq::Event event( zeroeq::uint128_t( 42 ));
    BOOST_CHECK_THROW( zeroeq::vocabulary::deserializeJSON( event ),
                       std::runtime_error );
}

BOOST_AUTO_TEST_CASE(serialization)
{
    const std::string message("test message");
    const zeroeq::Event& event = zeroeq::vocabulary::serializeEcho( message );
    const std::string& deserialized =
            zeroeq::vocabulary::deserializeEcho( event );
    BOOST_CHECK_EQUAL( message, deserialized );
}

BOOST_AUTO_TEST_CASE(json_serialization)
{
    const std::string json( "{\n"
                            "  \"message\": \"test message\"\n"
                            "}\n" );
    const zeroeq::Event& event = zeroeq::vocabulary::serializeJSON(
                                     zeroeq::vocabulary::EVENT_ECHO, json );
    const std::string& deserialized =
            zeroeq::vocabulary::deserializeJSON( event );
    BOOST_CHECK_EQUAL( json, deserialized );
}

BOOST_AUTO_TEST_CASE(invalid_json_serialization)
{
    const std::string json( "{\n"
                            "  \"wrongkey\": \"test message\"\n"
                            "}\n" );
    BOOST_CHECK_THROW( zeroeq::vocabulary::serializeJSON(
                           zeroeq::vocabulary::EVENT_ECHO, json ),
                       std::runtime_error );
}

BOOST_AUTO_TEST_CASE(json_with_binary_data_serialization)
{
    const std::string json( "{\n"
                            "  \"data\": \"UXQgaXMgZ3JlYXQh\"\n"
                            "}\n" );
    const zeroeq::Event& event =
            zeroeq::vocabulary::serializeJSON( zeroeqtest::EVENT_BINARY, json );
    const std::string& deserialized =
            zeroeq::vocabulary::deserializeJSON( event );
    BOOST_CHECK_EQUAL( json, deserialized );
}

namespace
{
zeroeq::Event serializeBinary( const std::string& msg )
{
    zeroeq::Event event( zeroeqtest::EVENT_BINARY );

    flatbuffers::FlatBufferBuilder& fbb = event.getFBB();
    zeroeqtest::BinaryBuilder builder( fbb );
    builder.add_data( fbb.CreateVector(
                    reinterpret_cast<const uint8_t*>(msg.data()), msg.size( )));
    fbb.Finish( builder.Finish( ));
    return event;
}
}

BOOST_AUTO_TEST_CASE(from_cpp_to_json_serialization)
{
    const zeroeq::Event& event = serializeBinary( "Hello there" );

    const std::string json( "{\n"
                            "  \"data\": \"SGVsbG8gdGhlcmU=\"\n"
                            "}\n" );
    const std::string& deserialized =
            zeroeq::vocabulary::deserializeJSON( event );
    BOOST_CHECK_EQUAL( json, deserialized );
}
