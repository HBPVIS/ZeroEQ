
/* Copyright (c) 2014, Human Brain Project
 *                     Stefan.Eilemann@epfl.ch
 *
 * This file is part of ZeroEQ (https://github.com/HBPVIS/zeq)
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

#define BOOST_TEST_MODULE zeq_new_event

#include "broker.h"

#include <tests/newEvent_generated.h>

#include <boost/bind.hpp>

namespace zeqtest
{
static const lunchbox::uint128_t EVENT_NEW(
    lunchbox::make_uint128( "zeq::TestNewEvent" ));
static const std::string message( "So long, and thanks for all the fish" );

zeq::Event serializeString( const std::string& string )
{
    ::zeq::Event event( EVENT_NEW );

    flatbuffers::FlatBufferBuilder& fbb = event.getFBB();
    auto data = fbb.CreateString( string );

    MessageBuilder builder( fbb );
    builder.add_message( data );
    fbb.Finish( builder.Finish( ));
    return event;
}

std::string deserializeString( const ::zeq::Event& event )
{
    BOOST_CHECK_EQUAL( event.getType(), EVENT_NEW );

    auto data = GetMessage( event.getData( ));
    return std::string( data->message()->c_str( ));
}

void onMessageEvent( const zeq::Event& event )
{
    BOOST_CHECK_EQUAL( deserializeString( event ), message );
}
}

BOOST_AUTO_TEST_CASE(test_new_event)
{
    lunchbox::RNG rng;
    const unsigned short port = (rng.get<uint16_t>() % 60000) + 1024;
    const std::string& portStr = boost::lexical_cast< std::string >( port );
    zeq::Subscriber subscriber( lunchbox::URI( "foo://localhost:" + portStr ));
    BOOST_CHECK( subscriber.registerHandler( zeqtest::EVENT_NEW,
                                  boost::bind( &zeqtest::onMessageEvent, _1 )));

    zeq::Publisher publisher( lunchbox::URI( "foo://*:" + portStr ));

    bool received = false;
    for( size_t i = 0; i < 10; ++i )
    {
        BOOST_CHECK(
            publisher.publish( zeqtest::serializeString( zeqtest::message )));

        if( subscriber.receive( 100 ))
        {
            received = true;
            break;
        }
    }
    BOOST_CHECK( received );
}
