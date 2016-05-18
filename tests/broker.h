
/* Copyright (c) 2014-2016, Human Brain Project
 *                          Daniel Nachbaur <daniel.nachbaur@epfl.ch>
 *                          Stefan.Eilemann@epfl.ch
 *                          Juan Hernando <jhernando@fi.upm.es>
 */

#include <zeroeq/detail/port.h>
#include <zeroeq/zeroeq.h>

#include <servus/serializable.h>
#include <servus/uri.h>
#include <boost/test/unit_test.hpp>
#include <string>

#ifdef ZEROEQ_USE_FLATBUFFERS
#include <tests/newEvent_generated.h>
#include <tests/newEvent_zeroeq_generated.h>
#include <tests/emptyEvent_generated.h>
#include <tests/emptyEvent_zeroeq_generated.h>
#endif

#ifdef _WIN32
#  include <process.h>
#  define getpid _getpid
#else
#  include <sys/types.h>
#  include <unistd.h>
#endif

namespace test
{
std::string buildUniqueSession()
{
    std::string name = std::string(
        boost::unit_test::framework::current_test_case().p_name );
    std::replace( name.begin(), name.end(), '_', '-' );
    return name + std::to_string( getpid( ));
}

zeroeq::URI buildURI( const std::string& hostname, const zeroeq::Publisher& to )
{
    assert( to.getURI().getPort( ));
    zeroeq::URI uri;
    uri.setHost( hostname );
    uri.setPort( to.getURI().getPort( ));
    return uri;
}

const std::string echoMessage( "So long, and thanks for all the fish!" );


void onEchoEvent( const zeroeq::FBEvent& event )
{
#ifdef ZEROEQ_USE_FLATBUFFERS
    BOOST_CHECK( event.getTypeIdentifier() == zeroeq::vocabulary::EVENT_ECHO );
    const std::string message = zeroeq::vocabulary::deserializeEcho( event );
    BOOST_CHECK_EQUAL( echoMessage, message );
#else
    (void)event;
#endif
}

void onEmptyEvent( const zeroeq::FBEvent& event )
{
#ifdef ZEROEQ_USE_FLATBUFFERS
    BOOST_CHECK_EQUAL( event.getTypeIdentifier(), ::zeroeqtest::EVENT_EMPTYEVENT );
    BOOST_CHECK_EQUAL( event.getSize(), 0 );
#else
    (void)event;
#endif
}

class Echo : public servus::Serializable
{
public:
    std::string getTypeName() const final { return "zeroeq::test::Echo"; }

    Echo() {}
    Echo( const std::string& message ) : _message( message ) {}
    const std::string& getMessage() const { return _message; }

    bool operator == ( const Echo& rhs ) const
    { return _message == rhs._message; }

private:
    bool _fromBinary( const void* data, const size_t ) final
    {
        _message = std::string( static_cast< const char* >( data ));
        return true;
    }

    Data _toBinary() const final
    {
        Data data;
        data.ptr = std::shared_ptr< const void >( _message.data(),
                                                  []( const void* ){} );
        data.size = _message.length();
        return data;
    }

    std::string _message;
};

class Empty : public servus::Serializable
{
public:
    std::string getTypeName() const final { return "zeroeq::test::EmptyEvent"; }

private:
    bool _fromBinary( const void*, const size_t ) final
    {
       return true;
    }

    Data _toBinary() const final
    {
        return Data();
    }
};


typedef std::shared_ptr< servus::Serializable > SerializablePtr;

SerializablePtr getFBEchoOutEvent( const std::string& message )
{
#ifdef ZEROEQ_USE_FLATBUFFERS
    ::zeroeq::FBEvent event = ::zeroeq::vocabulary::serializeEcho( message );
    SerializablePtr serializable( new ::zeroeq::FBEvent( std::move( event )));
    return serializable;
#else
    return SerializablePtr( new ::test::Echo( message ));
#endif
}

SerializablePtr getFBEchoInEvent( const ::zeroeq::EventFunc& eventFunc )
{
#ifdef ZEROEQ_USE_FLATBUFFERS
    return SerializablePtr( new ::zeroeq::FBEvent( ::zeroeq::vocabulary::EVENT_ECHO,
                                                   eventFunc ));
#else
    (void)eventFunc;
    return SerializablePtr( new ::test::Echo );
#endif
}

SerializablePtr getFBEmptyOutEvent()
{
#ifdef ZEROEQ_USE_FLATBUFFERS
    SerializablePtr serializable(
                new ::zeroeq::FBEvent( ::zeroeqtest::EVENT_EMPTYEVENT,
                                       ::zeroeq::EventFunc( )));
    return serializable;
#else
    return SerializablePtr( new ::test::Empty );
#endif
}

SerializablePtr getFBEmptyInEvent( const ::zeroeq::EventFunc& eventFunc )
{
#ifdef ZEROEQ_USE_FLATBUFFERS
    return SerializablePtr( new ::zeroeq::FBEvent( ::zeroeq::vocabulary::EVENT_ECHO,
                                                   eventFunc ));
#else
    (void)eventFunc;
    return SerializablePtr( new ::test::Empty );
#endif
}

}
