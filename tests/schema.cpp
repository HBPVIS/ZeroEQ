
/* Copyright (c) 2014, EPFL/Blue Brain Project
 *                     Daniel Nachbaur <daniel.nachbaur@epfl.ch>
 */

#include <zerobuf/data_generated.h>
#include <zerobuf/exit_generated.h>
#include <zerobuf/wildcard_generated.h>

#define BOOST_TEST_MODULE schema
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(data_message)
{
    flatbuffers::FlatBufferBuilder fbb;

    unsigned char buffer [] = { 0, 1, 2 };
    auto content = zerobuf::CreateContent( fbb, fbb.CreateString( "image.png" ),
                                           fbb.CreateVector( buffer, 3 ));
    std::vector< decltype( content ) > contents;
    contents.push_back( content );
    auto mloc = zerobuf::CreateData( fbb, fbb.CreateVector( contents ));
    fbb.Finish( mloc );

    auto data = zerobuf::GetData( fbb.GetBufferPointer( ));
    BOOST_CHECK( data );
    BOOST_CHECK_EQUAL( data->contents()->Length(), 1 );
}

BOOST_AUTO_TEST_CASE(exit_message)
{
    flatbuffers::FlatBufferBuilder fbb;

    auto mloc = zerobuf::CreateExit( fbb );
    fbb.Finish( mloc );

    auto exit = zerobuf::GetExit( fbb.GetBufferPointer( ));
    BOOST_CHECK( exit );
}

BOOST_AUTO_TEST_CASE(wildcard_message)
{
    flatbuffers::FlatBufferBuilder fbb;

    zerobuf::WildcardBuilder wb(fbb);
    auto path = zerobuf::CreatePath( fbb, fbb.CreateString( "hello" ));
    auto path2 = zerobuf::CreatePath( fbb, fbb.CreateString( "world" ));
    std::vector< decltype( path ) > paths;
    paths.push_back( path );
    paths.push_back( path2 );
    wb.add_paths( fbb.CreateVector( paths ));
    auto mloc = wb.Finish();
    fbb.Finish( mloc );

    auto wildcard = zerobuf::GetWildcard(fbb.GetBufferPointer( ));
    BOOST_CHECK_EQUAL( wildcard->paths()->Length(), paths.size( ));
    BOOST_CHECK_EQUAL( std::string(wildcard->paths()->Get(0)->name()->c_str()),
                       "hello");
    BOOST_CHECK_EQUAL( std::string(wildcard->paths()->Get(1)->name()->c_str()),
                       "world");
}
