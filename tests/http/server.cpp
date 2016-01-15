
/* Copyright (c) 2016, Human Brain Project
 *                     Stefan.Eilemann@epfl.ch
 */

#define BOOST_TEST_MODULE http_server

#include <zeq/http/server.h>
#include <zeq/subscriber.h>
#include <zeq/uri.h>
#include <zmq.h>
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

class Socket
{
public:
    Socket( const servus::URI& uri )
        : _ctx( ::zmq_ctx_new ( ))
        , _socket( ::zmq_socket( _ctx, ZMQ_STREAM ))
    {
        if( ::zmq_connect( _socket, std::to_string( uri ).c_str( )) == -1 )
            throw std::runtime_error( "Connect failed" );
    }

    ~Socket()
    {
        if( _socket )
            ::zmq_close( _socket );
        if( _ctx )
            ::zmq_ctx_destroy( _ctx );
    }

    std::string send( const std::string& request )
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

        if( ::zmq_recv( _socket, id, idSize, 0 ) != int( idSize ))
            throw std::runtime_error( "Recv failed" );

        std::string response( 256, '\0' );
        response.resize(
            ::zmq_recv( _socket, &response[0], response.size(), 0 ));
        return response;
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
}

BOOST_AUTO_TEST_CASE(get)
{
    bool running = true;
    zeq::http::Server server;
    Foo foo;
    server.register_( foo );

    std::thread thread( [ & ]() { while( running ) server.receive( 100 ); });

    Socket client( server.getURI( ));
    std::string reply = client.send( "GET /test/Foo HTTP/1.0\r\n\r\n" );
    BOOST_CHECK_EQUAL( reply,
                       std::string( "HTTP/1.0 200 OK\r\n\r\n" ) + jsonGet );
    reply = client.send( "GET /unknown HTTP/1.0\r\n\r\n" );
    BOOST_CHECK_EQUAL( reply,
                       std::string( "HTTP/1.0 404 Not Found\r\n\r\n" ));

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

    Socket client1( server1.getURI( ));
    Socket client2( server2.getURI( ));

    std::string reply = client1.send( "GET /test/Foo HTTP/1.0\r\n\r\n" );
    BOOST_CHECK_EQUAL( reply,
                       std::string( "HTTP/1.0 404 Not Found\r\n\r\n" ));

    reply = client2.send( "GET /test/Foo HTTP/1.0\r\n\r\n" );
    BOOST_CHECK_EQUAL( reply,
                       std::string( "HTTP/1.0 200 OK\r\n\r\n" ) + jsonGet );
    running = false;
    thread.join();
}

BOOST_AUTO_TEST_CASE(post)
{
    bool running = true;
    zeq::http::Server server;
    Foo foo;
    server.subscribe( foo );

    std::thread thread( [ & ]() { while( running ) server.receive( 100 ); });

    Socket client( server.getURI( ));
    std::string reply = client.send(
        std::string( "POST /test/Foo HTTP/1.0\r\n\r\n" ) + jsonPut );
    BOOST_CHECK_EQUAL( reply,
                       std::string( "HTTP/1.0 411 Length Required\r\n\r\n" ));

    reply = client.send(
        std::string( "POST /test/Foo HTTP/1.0\r\nContent-Length: " ) +
        std::to_string( jsonPut.length( )) + "\r\n\r\n" + jsonPut );
    BOOST_CHECK_EQUAL( reply, std::string( "HTTP/1.0 200 OK\r\n\r\n" ));

    reply = client.send( std::string(
        "POST /test/Foo HTTP/1.0\r\nContent-Length: 3\r\n\r\nFoo" ));
    BOOST_CHECK_EQUAL( reply, std::string( "HTTP/1.0 400 Bad Request\r\n\r\n"));

    reply = client.send(
        std::string( "POST /test/Bar HTTP/1.0\r\nContent-Length: " ) +
        std::to_string( jsonPut.length( )) + "\r\n\r\n" + jsonPut );
    BOOST_CHECK_EQUAL( reply,
                       std::string( "HTTP/1.0 404 Not Found\r\n\r\n" ));

    running = false;
    thread.join();
}

BOOST_AUTO_TEST_CASE(put)
{
    bool running = true;
    zeq::http::Server server;
    Foo foo;
    server.register_( foo );

    std::thread thread( [ & ]() { while( running ) server.receive( 100 ); });

    Socket client( server.getURI( ));
    std::string reply = client.send(
        std::string( "PUT /test/Foo HTTP/1.0\r\nContent-Length: " ) +
        std::to_string( jsonPut.length( )) + "\r\n\r\n" + jsonPut );
    BOOST_CHECK_EQUAL( reply,
                       std::string( "HTTP/1.0 405 Method Not Allowed\r\n\r\n"));

    running = false;
    thread.join();
}
