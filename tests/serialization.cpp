
/* Copyright (c) 2014-2015, Human Brain Project
 *                          Juan Hernando <jhernando@fi.upm.es>
 *                          Daniel Nachbaur <danielnachbaur@epfl.ch>
 */

#define BOOST_TEST_MODULE zeq_serialization

#include <zeq/zeq.h>

#include <tests/binary_generated.h>
#include <tests/binary_zeq_generated.h>

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(test_serialization)
{
    const std::string message("test message");
    const zeq::Event& event = zeq::vocabulary::serializeEcho( message );
    const std::string& deserialized = zeq::vocabulary::deserializeEcho( event );
    BOOST_CHECK_EQUAL( message, deserialized );
}

BOOST_AUTO_TEST_CASE(test_json_serialization)
{
    const std::string json( "{\n"
                            "  \"message\": \"test message\"\n"
                            "}\n" );
    const zeq::Event& event =
            zeq::vocabulary::serializeJSON( zeq::vocabulary::EVENT_ECHO, json );
    const std::string& deserialized = zeq::vocabulary::deserializeJSON( event );
    BOOST_CHECK_EQUAL( json, deserialized );
}

BOOST_AUTO_TEST_CASE(test_json_with_binary_data_serialization)
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

BOOST_AUTO_TEST_CASE(test_from_cpp_to_json_serialization)
{
    const zeq::Event& event = serializeBinary( "Hello there" );

    const std::string json( "{\n"
                            "  \"data\": \"SGVsbG8gdGhlcmU=\"\n"
                            "}\n" );
    const std::string& deserialized = zeq::vocabulary::deserializeJSON( event );
    BOOST_CHECK_EQUAL( json, deserialized );
}
