
/* Copyright (c) 2016, Human Brain Project
 *                     Stefan.Eilemann@epfl.ch
 */

#define BOOST_TEST_MODULE http_server

#include <zeq/http/server.h>
#include <zeq/subscriber.h>
#include <zeq/uri.h>
#include <zmq.h>
#include <servus/serializable.h>
#include <servus/uri.h>
#include <boost/test/unit_test.hpp>
#include <thread>

namespace
{
std::string jsonGet( "Not JSON, but I just want to see that the data is a-ok" );
std::string jsonPut( "See what my stepbrother jsonGet says" );

class Foo : public servus::Serializable
{
private:
    std::string getTypeName() const final { return "test::Foo"; }
    virtual zeq::uint128_t getTypeIdentifier() const final
        { return servus::make_uint128( getTypeName( )); }

    bool _fromJSON( const std::string& json ) final
    {
        return jsonPut == json;
    }

    std::string _toJSON() const final
    {
        return jsonGet;
    }
};

class Client
{
public:
    Client( const servus::URI& uri )
        : _ctx( ::zmq_ctx_new ( ))
        , _socket( ::zmq_socket( _ctx, ZMQ_STREAM ))
    {
        if( ::zmq_connect( _socket, std::to_string( uri ).c_str( )) == -1 )
            throw std::runtime_error( "Connect failed" );
    }

    ~Client()
    {
        if( _socket )
            ::zmq_close( _socket );
        if( _ctx )
            ::zmq_ctx_destroy( _ctx );
    }

    void test( const std::string& request, const std::string& expected )
    {
        // Get server identity
        uint8_t id[256];
        size_t idSize = sizeof( id );
        BOOST_CHECK_EQUAL( zmq_getsockopt( _socket, ZMQ_IDENTITY, id, &idSize ),
                           0 );

        if( ::zmq_send( _socket, id, idSize, ZMQ_SNDMORE ) != int( idSize ) ||
            ::zmq_send( _socket, request.c_str(), request.length(), 0 ) !=
            int( request.length( )))
        {
            throw std::runtime_error( "Send failed" );
        }

        std::string response;
        while( response.size() < expected.size( ))
        {
            if( ::zmq_recv( _socket, id, idSize, 0 ) != int( idSize ))
                throw std::runtime_error( "Recv failed" );

            char msg[256];
            const int read = ::zmq_recv( _socket, msg, sizeof( msg ), 0 );
            BOOST_REQUIRE( read > 0 );
            response.append( msg, read );
        }

        BOOST_CHECK_EQUAL( response, expected );
    }

private:
    void* _ctx;
    void* _socket;
};

}

BOOST_AUTO_TEST_CASE(construction)
{
    zeq::http::Server server1;
    BOOST_CHECK_NE( server1.getURI().getHost(), "" );
    BOOST_CHECK_NE( server1.getURI().getHost(), "*" );
    BOOST_CHECK_NE( server1.getURI().getPort(), 0 );

    const zeq::URI uri( "tcp://" );
    zeq::http::Server server2( uri );
    zeq::http::Server server3( uri );
    BOOST_CHECK_NE( server2.getURI(), server3.getURI( ));
    BOOST_CHECK_NE( server2.getURI().getPort(), 0 );

    BOOST_CHECK_THROW( zeq::http::Server( server2.getURI( )),
                       std::runtime_error );
}

BOOST_AUTO_TEST_CASE(registration)
{
    zeq::http::Server server;
    Foo foo;
    BOOST_CHECK( server.register_( foo ));
    BOOST_CHECK( !server.register_( foo ));
    BOOST_CHECK( server.unregister( foo ));
    BOOST_CHECK( !server.unregister( foo ));

    BOOST_CHECK( server.subscribe( foo ));
    BOOST_CHECK( !server.subscribe( foo ));
    BOOST_CHECK( server.unsubscribe( foo ));
    BOOST_CHECK( !server.unsubscribe( foo ));

    BOOST_CHECK( server.add( foo ));
    BOOST_CHECK( !server.add( foo ));
    BOOST_CHECK( server.remove( foo ));
    BOOST_CHECK( !server.remove( foo ));
}

BOOST_AUTO_TEST_CASE(get)
{
    bool running = true;
    zeq::http::Server server;
    Foo foo;
    server.register_( foo );

    std::thread thread( [ & ]() { while( running ) server.receive( 100 ); });

    Client client( server.getURI( ));
    client.test( "GET /test/Foo HTTP/1.0\r\n\r\n",
                 std::string( "HTTP/1.0 200 OK\r\nContent-Length: 54\r\n\r\n" ) +
                 jsonGet );
    client.test( "GET /unknown HTTP/1.0\r\n\r\n",
                 std::string( "HTTP/1.0 404 Not Found\r\nContent-Length: 0\r\n\r\n" ));

    running = false;
    thread.join();
}

BOOST_AUTO_TEST_CASE(shared)
{
    bool running = true;
    zeq::Subscriber subscriber;
    zeq::http::Server server1( subscriber );
    zeq::http::Server server2( server1 );
    Foo foo;
    server2.register_( foo );

    std::thread thread( [ & ]() { while( running ) subscriber.receive( 100 );});

    Client client1( server1.getURI( ));
    Client client2( server2.getURI( ));

    client1.test( "GET /test/Foo HTTP/1.0\r\n\r\n",
                  std::string( "HTTP/1.0 404 Not Found\r\nContent-Length: 0\r\n\r\n" ));
    client2.test( "GET /test/Foo HTTP/1.0\r\n\r\n",
                  std::string( "HTTP/1.0 200 OK\r\nContent-Length: 54\r\n\r\n" ) +
                  jsonGet );
    running = false;
    thread.join();
}

BOOST_AUTO_TEST_CASE(put)
{
    bool running = true;
    zeq::http::Server server;
    Foo foo;
    server.subscribe( foo );

    std::thread thread( [ & ]() { while( running ) server.receive( 100 ); });

    Client client( server.getURI( ));
    client.test( std::string( "PUT /test/Foo HTTP/1.0\r\n\r\n" ) + jsonPut,
                 std::string( "HTTP/1.0 411 Length Required\r\nContent-Length: 0\r\n\r\n" ));
    client.test( std::string( "PUT /test/Foo HTTP/1.0\r\nContent-Length: " ) +
                 std::to_string( jsonPut.length( )) + "\r\n\r\n" + jsonPut,
                 std::string( "HTTP/1.0 200 OK\r\nContent-Length: 0\r\n\r\n" ));
    client.test(
        std::string( "PUT /test/Foo HTTP/1.0\r\nContent-Length: 3\r\n\r\nFoo" ),
        std::string( "HTTP/1.0 400 Bad Request\r\nContent-Length: 0\r\n\r\n" ));
    client.test( std::string( "PUT /test/Bar HTTP/1.0\r\nContent-Length: " ) +
                 std::to_string( jsonPut.length( )) + "\r\n\r\n" + jsonPut,
                 std::string( "HTTP/1.0 404 Not Found\r\nContent-Length: 0\r\n\r\n" ));

    running = false;
    thread.join();
}

BOOST_AUTO_TEST_CASE(post)
{
    bool running = true;
    zeq::http::Server server;
    Foo foo;
    server.register_( foo );

    std::thread thread( [ & ]() { while( running ) server.receive( 100 ); });

    Client client( server.getURI( ));
    client.test( std::string( "POST /test/Foo HTTP/1.0\r\nContent-Length: " ) +
                 std::to_string( jsonPut.length( )) + "\r\n\r\n" + jsonPut,
                 std::string( "HTTP/1.0 405 Method Not Allowed\r\nContent-Length: 0\r\n\r\n"));

    running = false;
    thread.join();
}

BOOST_AUTO_TEST_CASE(largeGet)
{
    bool running = true;
    zeq::http::Server server;
    Foo foo;
    server.register_( foo );

    std::thread thread( [ & ]() { while( running ) server.receive( 100 ); });

    Client client( server.getURI( ));
    client.test( "GET" + std::string( 4096, ' ' ) + "/test/Foo HTTP/1.0\r\n\r\n",
                 std::string( "HTTP/1.0 200 OK\r\nContent-Length: 54\r\n\r\n" ) +
                 jsonGet );

    running = false;
    thread.join();
}

BOOST_AUTO_TEST_CASE(garbage)
{
    bool running = true;
    zeq::http::Server server;
    std::thread thread( [ & ]() { while( running ) server.receive( 100 ); });

    Client client( server.getURI( ));
    client.test( "ramble mumble foo bar", "" );

    running = false;
    thread.join();
}
