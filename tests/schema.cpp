
/* Copyright (c) 2014, EPFL/Blue Brain Project
 *                     Daniel Nachbaur <daniel.nachbaur@epfl.ch>
 */

#include <zerobuf/data_generated.h>
#include <zerobuf/exit_generated.h>
#include <zerobuf/wildcard_generated.h>

#include <capnp/message.h>
#include <capnp/serialize-packed.h>
#include <capnzero/wildcard.capnp.h>

#define BOOST_TEST_MODULE schema
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(data_message)
{
    flatbuffers::FlatBufferBuilder message;

    unsigned char buffer [] = { 0, 1, 2 };
    auto content = zerobuf::CreateContent( message, message.CreateString( "image.png" ),
                                           message.CreateVector( buffer, 3 ));
    std::vector< decltype( content ) > contents;
    contents.push_back( content );
    auto mloc = zerobuf::CreateData( message, message.CreateVector( contents ));
    message.Finish( mloc );

    auto data = zerobuf::GetData( message.GetBufferPointer( ));
    BOOST_CHECK( data );
    BOOST_CHECK_EQUAL( data->contents()->Length(), 1 );
}

BOOST_AUTO_TEST_CASE(exit_message)
{
    flatbuffers::FlatBufferBuilder message;

    auto mloc = zerobuf::CreateExit( message );
    message.Finish( mloc );

    auto exit = zerobuf::GetExit( message.GetBufferPointer( ));
    BOOST_CHECK( exit );
}

BOOST_AUTO_TEST_CASE(wildcard_message_flatbuffers)
{
    // compose message
    flatbuffers::FlatBufferBuilder message;
    zerobuf::WildcardBuilder wildcard( message );
    auto path = zerobuf::CreatePath( message, message.CreateString( "hello" ));
    auto path2 = zerobuf::CreatePath( message, message.CreateString( "world" ));
    message.StartVector( 2, sizeof( decltype( path )));
    message.PushElement( path2 );
    message.PushElement( path );
    wildcard.add_paths( message.EndVector( 2 ));
    message.Finish( wildcard.Finish( ));

    // send
    const uint8_t* data = message.GetBufferPointer();

    // receive
    auto recvWildcard = zerobuf::GetWildcard( data );

    // read message
    BOOST_CHECK_EQUAL( recvWildcard->paths()->Length(), 2 );
    BOOST_CHECK_EQUAL( std::string(recvWildcard->paths()->Get(0)->name()->c_str()),
                       "hello");
    BOOST_CHECK_EQUAL( std::string(recvWildcard->paths()->Get(1)->name()->c_str()),
                       "world");
}

BOOST_AUTO_TEST_CASE(wildcard_message_capnp)
{
    // compose message
    capnp::MallocMessageBuilder message;
    auto wildcard = message.initRoot< Wildcard >();
    auto paths = wildcard.initPaths( 2 );
    paths.set( 0, "hello" );
    paths.set( 1, "world" );

    // send
    auto array = messageToFlatArray( message );
    capnp::word* data = array.asPtr().begin();
    uint64_t size = computeSerializedSizeInWords( message ) * sizeof(capnp::word);

    // receive
    capnp::FlatArrayMessageReader reader( kj::Array< capnp::word >( data, size,
                                                                    kj::NullArrayDisposer( )));
    auto recvWildcard = reader.getRoot< Wildcard >();

    // read message
    BOOST_CHECK_EQUAL( recvWildcard.getPaths().size(), paths.size( ));
    BOOST_CHECK_EQUAL( std::string(recvWildcard.getPaths()[0]),
                       "hello");
    BOOST_CHECK_EQUAL( std::string(recvWildcard.getPaths()[1]),
                       "world");
}
