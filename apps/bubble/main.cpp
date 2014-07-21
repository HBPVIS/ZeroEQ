
/* Copyright (c) 2014, BBP/EPFL
 *                     Stefan.Eilemann@epfl.ch
 *
 * This file is part of Hello (https://github.com/BlueBrain/Hello)
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 * - Neither the name of Eyescale Software GmbH nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */


#include <zerobuf/data_generated.h>
#include <zerobuf/exit_generated.h>
#include <zerobuf/expand_generated.h>
#include <zerobuf/wildcard_generated.h>
#include <zerobuf/version.h>

#include <lunchbox/file.h>
#include <lunchbox/memoryMap.h>
#include <boost/program_options.hpp>
#include <iostream>
#include <zmq.h>
namespace po = boost::program_options;

enum Role
{
    WILDCARDER = 5555,
    EXPANDER,
    READER,
    HASHER,
    UNKNOWN
};

template< uint32_t role >
class Bubble
{
private:
    void* _subscriber;
    void* _exitSubscriber;
    void* _publisher;
    void* _context;
public:
    Bubble()
        : _subscriber( 0 )
        , _exitSubscriber( 0 )
        , _publisher( 0 )
        , _context( zmq_ctx_new( ))
    {
        const std::string& previousRole = boost::lexical_cast< std::string >( role-1 );
        const std::string& roleString = boost::lexical_cast< std::string >( role );
        const std::string& exitString = boost::lexical_cast< std::string >( HASHER );
        if( role != WILDCARDER )
        {
            _subscriber = zmq_socket( _context, ZMQ_SUB );
            zmq_connect( _subscriber, std::string( "tcp://localhost:" + previousRole ).c_str( ));
            zmq_setsockopt( _subscriber, ZMQ_SUBSCRIBE, "", 0 );
        }

        _publisher = zmq_socket( _context, ZMQ_PUB );
        zmq_bind( _publisher, std::string( "tcp://*:" + roleString ).c_str( ));

        _exitSubscriber = zmq_socket( _context, ZMQ_SUB );
        zmq_connect( _exitSubscriber, std::string( "tcp://localhost:" + exitString ).c_str( ));
        zmq_setsockopt( _exitSubscriber, ZMQ_SUBSCRIBE, "", 0 );
    }

    ~Bubble()
    {
        zmq_close( _exitSubscriber );
        zmq_close( _publisher );
        zmq_close( _subscriber );
        zmq_ctx_destroy( _context );
    }

    void process( uint8_t* buf );

    void receive()
    {
        //  Process messages from both sockets
        for( ;; )
        {
            uint64_t msgSize;
            zmq_pollitem_t items[2] = {{ _exitSubscriber, 0, ZMQ_POLLIN, 0 },
                                       { _subscriber, 0, ZMQ_POLLIN, 0 }};

            zmq_poll( items, role == WILDCARDER ? 1 : 2, role == WILDCARDER ? 1000 : -1 );

            // exit, Junge!
            if( items[0].revents & ZMQ_POLLIN )
            {
                if( zmq_recv( _exitSubscriber, &msgSize, sizeof( msgSize ), 0) == -1 )
                    continue;

                uint8_t* buf = (uint8_t*)alloca( msgSize );
                if( zmq_recv( _exitSubscriber, buf, msgSize, 0 ) == -1 )
                    continue;

                break;
            }

            if( role != WILDCARDER )
            {
                if( items[1].revents & ZMQ_POLLIN )
                {
                    if( zmq_recv( _subscriber, &msgSize, sizeof( msgSize ), 0) == -1 )
                        continue;

                    uint8_t* buf = (uint8_t*)alloca( msgSize );
                    if( zmq_recv( _subscriber, buf, msgSize, 0 ) == -1 )
                        continue;

                    process( buf );
                }
            }
            else
            {
                flatbuffers::FlatBufferBuilder fbb;

                zerobuf::WildcardBuilder wb(fbb);
                auto path = zerobuf::CreatePath( fbb, fbb.CreateString( ".*" ));
                std::vector< decltype( path ) > paths;
                paths.push_back( path );
                wb.add_paths( fbb.CreateVector( paths ));
                auto mloc = wb.Finish();
                fbb.Finish( mloc );

                const uint64_t wcms = fbb.GetSize();
                zmq_send( _publisher, &wcms, sizeof(wcms), 0 );
                zmq_send( _publisher, fbb.GetBufferPointer(), fbb.GetSize(), 0 );
            }
        }
    }
};

template<>
void Bubble< WILDCARDER >::process( uint8_t* )
{
}

template<>
void Bubble< EXPANDER >::process( uint8_t* buffer )
{
    flatbuffers::FlatBufferBuilder fbb;
    zerobuf::ExpandBuilder eb(fbb);
    auto path = zerobuf::CreateFile( fbb, fbb.CreateString( "" ));
    std::vector< decltype( path ) > paths;

    auto wildcard = zerobuf::GetWildcard( buffer );
    for( auto i : *wildcard->paths( ))
    {
        const lunchbox::Strings& files =
            lunchbox::searchDirectory( ".", i->name()->c_str( ));
        for( auto file : files )
        {
            std::cout << "EXPAND: " << file << std::endl;
            path = zerobuf::CreateFile( fbb, fbb.CreateString( file.c_str( )));
            paths.push_back( path );
        }
    }

    eb.add_files( fbb.CreateVector( paths ));
    auto mloc = eb.Finish();
    fbb.Finish( mloc );

    const uint64_t msgSize = fbb.GetSize();
    zmq_send( _publisher, &msgSize, sizeof(msgSize), 0 );
    zmq_send( _publisher, fbb.GetBufferPointer(), fbb.GetSize(), 0 );
}

template<>
void Bubble< READER >::process( uint8_t* buffer )
{
    flatbuffers::FlatBufferBuilder fbb;
    auto content = zerobuf::CreateContent( fbb, fbb.CreateString( "" ));
    std::vector< decltype( content ) > contents;

    auto expand = zerobuf::GetExpand( buffer );
    for( auto i : *expand->files( ))
    {
        auto path = std::string(i->name()->c_str());
        lunchbox::MemoryMap file( path );
        content = zerobuf::CreateContent( fbb, fbb.CreateString( path ),
                                               fbb.CreateVector( (uint8_t*)( file.getAddress() ), file.getSize() ));

        contents.push_back( content );
    }

    auto mloc = zerobuf::CreateData( fbb, fbb.CreateVector( contents ));
    fbb.Finish( mloc );

    const uint64_t msgSize = fbb.GetSize();
    zmq_send( _publisher, &msgSize, sizeof(msgSize), 0 );
    zmq_send( _publisher, fbb.GetBufferPointer(), fbb.GetSize(), 0 );
}

template<>
void Bubble< HASHER >::process( uint8_t* buffer )
{
    auto data = zerobuf::GetData( buffer );
    for( auto i : *data->contents( ))
    {
        std::cout << "Name: " << i->name()->c_str() << ", buffer size: "
                  << i->buffer()->Length() << std::endl;
    }

    flatbuffers::FlatBufferBuilder fbb;
    auto mloc = zerobuf::CreateExit( fbb );
    fbb.Finish( mloc );

    const uint64_t msgSize = fbb.GetSize();
    zmq_send( _publisher, &msgSize, sizeof(msgSize), 0 );
    zmq_send( _publisher, fbb.GetBufferPointer(), fbb.GetSize(), 0 );
}

int main( int argc, char *argv[] )
{
    uint32_t role = UNKNOWN;

    // Arguments parsing
    po::variables_map vm;
    po::options_description desc( "Supported options" );
    desc.add_options()
        ( "help,h", "show help message." )
        ( "version,v", "Show program name/version banner and exit." )
        ( "role,r", po::value<uint32_t>(&role), "What do I do?" );
    po::store( parse_command_line( argc, argv, desc ), vm );
    po::notify( vm );

    if( vm.count( "help" ))
    {
        std::cout << desc << std::endl;
        return EXIT_SUCCESS;
    }

    if( vm.count( "version" ))
    {
        std::cout << "HelloRunner version " << zerobuf::Version::getString()
                  << std::endl
                  << "Copyright (c) BBP/EPFL 2014." << std::endl;
        return EXIT_SUCCESS;
    }

    if( role == UNKNOWN )
        return EXIT_FAILURE;

    switch( role )
    {
    case WILDCARDER:
    {
        Bubble< WILDCARDER > bubble;
        bubble.receive();
        break;
    }
    case EXPANDER:
    {
        Bubble< EXPANDER > bubble;
        bubble.receive();
        break;
    }
    case READER:
    {
        Bubble< READER > bubble;
        bubble.receive();
        break;
    }
    case HASHER:
    {
        Bubble< HASHER > bubble;
        bubble.receive();
        break;
    }
    }

    return EXIT_SUCCESS;
}
