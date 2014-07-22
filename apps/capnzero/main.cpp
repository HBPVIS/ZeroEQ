
/* Copyright (c) 2014, BBP/EPFL
 *                     Daniel.Nachbaur@epfl.ch
 *
 * This file is part of VisEDA (https://github.com/HBPVIS/VisEDA)
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


#include <capnzero/data.capnp.h>
#include <capnzero/expand.capnp.h>
#include <capnzero/exit.capnp.h>
#include <capnzero/wildcard.capnp.h>
#include <capnp/message.h>
#include <capnp/serialize-packed.h>
#include <viseda/version.h>

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

    typedef kj::Array< ::capnp::word > Array;
    typedef kj::ArrayPtr< ::capnp::word > ArrayPtr;

    void _sendMessage( ::capnp::MallocMessageBuilder& message )
    {
        auto array = messageToFlatArray( message );

        const uint64_t arraySize = computeSerializedSizeInWords( message ) * sizeof( ::capnp::word);
        zmq_send( _publisher, &arraySize, sizeof(arraySize), 0 );
        zmq_send( _publisher, array.asPtr().begin(), arraySize, 0 );
    }

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

    void process( ArrayPtr buffer );

    void receive()
    {
        //  Process messages from both sockets
        for( ;; )
        {
            uint64_t msgSize;
            zmq_pollitem_t items[2] = {{ _exitSubscriber, 0, ZMQ_POLLIN, 0 },
                                       { _subscriber, 0, ZMQ_POLLIN, 0 }};

            zmq_poll( items, role == WILDCARDER ? 1 : 2, role == WILDCARDER ? 1000 : -1 );

            // exit channel
            if( items[0].revents & ZMQ_POLLIN )
            {
                if( zmq_recv( _exitSubscriber, &msgSize, sizeof( msgSize ), 0) == -1 )
                    continue;

                uint64_t* buf = (uint64_t*)alloca( msgSize );
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

                    ::capnp::word* buf = (::capnp::word*)alloca( msgSize );
                    if( zmq_recv( _subscriber, buf, msgSize, 0 ) == -1 )
                        continue;


                    process( Array( buf, msgSize, kj::NullArrayDisposer( )));
                }
            }
            else
            {
                ::capnp::MallocMessageBuilder message;
                auto wildcard = message.initRoot< Wildcard >();
                auto paths = wildcard.initPaths( 1 );
                paths.set( 0, ".*" );

                _sendMessage( message );
            }
        }
    }
};

template<>
void Bubble< WILDCARDER >::process( ArrayPtr )
{
}

template<>
void Bubble< EXPANDER >::process( ArrayPtr buffer )
{
    ::capnp::FlatArrayMessageReader reader( buffer );
    auto wildcard = reader.getRoot< Wildcard >();

    ::capnp::MallocMessageBuilder message;
    auto expand = message.initRoot< Expand >();

    uint j = 0;
    for( auto i : wildcard.getPaths( ))
    {
        auto path = std::string(i);
        const lunchbox::Strings& filesNames = lunchbox::searchDirectory( ".", path );
        auto files = expand.initFiles( filesNames.size( )); //hack
        for( auto fileName : filesNames )
        {
            std::cout << "EXPAND: " << fileName << std::endl;
            files.set( j++, fileName );
        }
        break; //hack
    }

    _sendMessage( message );
}

template<>
void Bubble< READER >::process( ArrayPtr buffer )
{
    ::capnp::FlatArrayMessageReader reader( buffer );
    auto expand = reader.getRoot< Expand >();

    ::capnp::MallocMessageBuilder message;
    auto data = message.initRoot<TheData>();

    auto contents = data.initContents( expand.getFiles().size( ));
    uint j = 0;
    for( auto i : expand.getFiles( ))
    {
        auto fileName = std::string(i);
        lunchbox::MemoryMap file( fileName );

        auto&& content = contents[j];
        auto theBuffer = content.initBuffer( file.getSize( ));
        content.setName( fileName );
        content.setBuffer( theBuffer );
        ++j;
    }

    _sendMessage( message );
}

template<>
void Bubble< HASHER >::process( ArrayPtr buffer )
{
    ::capnp::FlatArrayMessageReader reader( buffer );
    auto data = reader.getRoot< TheData >();

    for( auto i : data.getContents( ))
    {
        std::cout << "Name: " << std::string(i.getName()) << ", buffer size: "
                  << i.getBuffer().size() << std::endl;
    }

    ::capnp::MallocMessageBuilder message;
    message.initRoot< Exit >();

    _sendMessage( message );
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
        std::cout << "HelloRunner version " << viseda::Version::getString()
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
