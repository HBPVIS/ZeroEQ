
/* Copyright (c) 2014-2015, Human Brain Project
 *                          Juan Hernando <jhernando@fi.upm.es>
 *                          Daniel Nachbaur <danielnachbaur@epfl.ch>
 */

#define BOOST_TEST_MODULE zeq_serialization

#include <zeq/zeq.h>

#include <zeq/vocabulary_generated.h>
#include <zeq/vocabulary_zeq_generated.h>
#include <zeq/eventDescriptor.h>

#include <tests/binary_generated.h>
#include <tests/binary_zeq_generated.h>

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(vocabulary_serialization)
{
    zeq::EventDescriptors vocabulary;

    zeq::EventDescriptor onlySubscribedEvent( "testName01", zeq::uint128_t( 1 ),
                                           "testSchema01", zeq::SUBSCRIBER );
    zeq::EventDescriptor onlyPublishedEvent( "testName02", zeq::uint128_t( 2 ),
                                           "testSchema02", zeq::PUBLISHER );
    zeq::EventDescriptor bidirectionalEvent( "testName03",  zeq::uint128_t( 3 ),
                                           "testSchema03", zeq::BIDIRECTIONAL );
    vocabulary.push_back( std::move(onlySubscribedEvent) );
    vocabulary.push_back( std::move(onlyPublishedEvent) );
    vocabulary.push_back( std::move(bidirectionalEvent) );

    const zeq::Event& event = zeq::vocabulary::serializeVocabulary( vocabulary);

    zeq::EventDescriptors deserialized_vocabulary =
                                zeq::vocabulary::deserializeVocabulary( event );

    BOOST_CHECK_EQUAL( deserialized_vocabulary[0].getRestName(), "testName01" );
    BOOST_CHECK_EQUAL( deserialized_vocabulary[0].getEventType(),
                       zeq::uint128_t( 1 ));
    BOOST_CHECK_EQUAL( deserialized_vocabulary[0].getSchema(), "testSchema01" );
    BOOST_CHECK_EQUAL( deserialized_vocabulary[0].getEventDirection(),
                       zeq::SUBSCRIBER );

    BOOST_CHECK_EQUAL( deserialized_vocabulary[1].getRestName(), "testName02" );
    BOOST_CHECK_EQUAL( deserialized_vocabulary[1].getEventType(),
                       zeq::uint128_t( 2 ));
    BOOST_CHECK_EQUAL( deserialized_vocabulary[1].getSchema(), "testSchema02" );
    BOOST_CHECK_EQUAL( deserialized_vocabulary[1].getEventDirection(),
                       zeq::PUBLISHER );

    BOOST_CHECK_EQUAL( deserialized_vocabulary[2].getRestName(), "testName03" );
    BOOST_CHECK_EQUAL( deserialized_vocabulary[2].getEventType(),
                       zeq::uint128_t( 3 ));
    BOOST_CHECK_EQUAL( deserialized_vocabulary[2].getSchema(), "testSchema03" );
    BOOST_CHECK_EQUAL( deserialized_vocabulary[2].getEventDirection(),
                       zeq::BIDIRECTIONAL );
}

BOOST_AUTO_TEST_CASE(request_serialization)
{
    const zeq::uint128_t eventType( zeq::vocabulary::EVENT_ECHO );
    const zeq::Event& requestEvent =
            zeq::vocabulary::serializeRequest( eventType );
    const zeq::uint128_t deserializedEventType =
            zeq::vocabulary::deserializeRequest( requestEvent );
    BOOST_CHECK_EQUAL( eventType, deserializedEventType );
}

BOOST_AUTO_TEST_CASE(event_registration)
{
    const std::string schema = "table Test { message:string; } root_type Test;";
    const zeq::uint128_t eventType( 42 );
    zeq::vocabulary::registerEvent( eventType, schema );

    const std::string json( "{\n"
                            "  \"message\": \"test message\"\n"
                            "}\n" );
    const zeq::Event& event = zeq::vocabulary::serializeJSON( eventType, json );
    const std::string& deserialized = zeq::vocabulary::deserializeJSON( event );
    BOOST_CHECK_EQUAL( json, deserialized );
}

BOOST_AUTO_TEST_CASE(invalid_event_registration)
{
    const std::string schema = "wrong Test { message:string; } root_type Test;";
    const zeq::uint128_t eventType( 42 );
    zeq::vocabulary::registerEvent( eventType, schema );

    zeq::Event event( zeq::uint128_t( 42 ));
    BOOST_CHECK_THROW( zeq::vocabulary::deserializeJSON( event ),
                       std::runtime_error );
}

BOOST_AUTO_TEST_CASE(serialization)
{
    const std::string message("test message");
    const zeq::Event& event = zeq::vocabulary::serializeEcho( message );
    const std::string& deserialized = zeq::vocabulary::deserializeEcho( event );
    BOOST_CHECK_EQUAL( message, deserialized );
}

BOOST_AUTO_TEST_CASE(json_serialization)
{
    const std::string json( "{\n"
                            "  \"message\": \"test message\"\n"
                            "}\n" );
    const zeq::Event& event =
            zeq::vocabulary::serializeJSON( zeq::vocabulary::EVENT_ECHO, json );
    const std::string& deserialized = zeq::vocabulary::deserializeJSON( event );
    BOOST_CHECK_EQUAL( json, deserialized );
}

BOOST_AUTO_TEST_CASE(invalid_json_serialization)
{
    const std::string json( "{\n"
                            "  \"wrongkey\": \"test message\"\n"
                            "}\n" );
    BOOST_CHECK_THROW(
        zeq::vocabulary::serializeJSON( zeq::vocabulary::EVENT_ECHO, json ),
        std::runtime_error );
}

BOOST_AUTO_TEST_CASE(json_with_binary_data_serialization)
{
    const std::string json( "{\n"
                            "  \"data\": \"UXQgaXMgZ3JlYXQh\"\n"
                            "}\n" );
    const zeq::Event& event =
            zeq::vocabulary::serializeJSON( zeqtest::EVENT_BINARY, json );
    const std::string& deserialized = zeq::vocabulary::deserializeJSON( event );
    BOOST_CHECK_EQUAL( json, deserialized );
}

namespace
{
zeq::Event serializeBinary( const std::string& msg )
{
    zeq::Event event( zeqtest::EVENT_BINARY );

    flatbuffers::FlatBufferBuilder& fbb = event.getFBB();
    zeqtest::BinaryBuilder builder( fbb );
    builder.add_data( fbb.CreateVector(
                    reinterpret_cast<const uint8_t*>(msg.data()), msg.size( )));
    fbb.Finish( builder.Finish( ));
    return event;
}
}

BOOST_AUTO_TEST_CASE(from_cpp_to_json_serialization)
{
    const zeq::Event& event = serializeBinary( "Hello there" );

    const std::string json( "{\n"
                            "  \"data\": \"SGVsbG8gdGhlcmU=\"\n"
                            "}\n" );
    const std::string& deserialized = zeq::vocabulary::deserializeJSON( event );
    BOOST_CHECK_EQUAL( json, deserialized );
}
