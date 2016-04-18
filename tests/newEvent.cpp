
/* Copyright (c) 2014-2015, Human Brain Project
 *                          Stefan.Eilemann@epfl.ch
 *                          Daniel.Nachbaur@epfl.ch
 *
 * This file is part of ZeroEQ (https://github.com/HBPVIS/ZeroEQ)
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

#define BOOST_TEST_MODULE zeroeq_new_event

#include "broker.h"

#include <tests/newEvent_generated.h>
#include <tests/newEvent_zeroeq_generated.h>

namespace zeroeqtest
{
static const std::string message( "So long, and thanks for all the fish" );

zeroeq::Event serializeString( const std::string& string )
{
    ::zeroeq::Event event( EVENT_NEWEVENT );

    flatbuffers::FlatBufferBuilder& fbb = event.getFBB();
    auto data = fbb.CreateString( string );

    NewEventBuilder builder( fbb );
    builder.add_message( data );
    fbb.Finish( builder.Finish( ));
    return event;
}

std::string deserializeString( const ::zeroeq::Event& event )
{
    BOOST_CHECK_EQUAL( event.getType(), EVENT_NEWEVENT );

    auto data = GetNewEvent( event.getData( ));
    return std::string( data->message()->c_str( ));
}

void onMessageEvent( const zeroeq::Event& event )
{
    BOOST_CHECK_EQUAL( deserializeString( event ), message );
}
}

BOOST_AUTO_TEST_CASE(new_event)
{
    zeroeq::Publisher publisher( zeroeq::NULL_SESSION );
    zeroeq::Subscriber subscriber( zeroeq::URI( publisher.getURI( )));

    ::zeroeq::Event newEvent( ::zeroeqtest::EVENT_NEWEVENT, std::bind(
                                                        &zeroeqtest::onMessageEvent,
                                                        std::placeholders::_1 ));
    BOOST_CHECK( subscriber.subscribe( newEvent ));

    bool received = false;
    for( size_t i = 0; i < 10; ++i )
    {
        BOOST_CHECK( publisher.publish( zeroeqtest::serializeString( zeroeqtest::message )));

        if( subscriber.receive( 100 ))
        {
            received = true;
            break;
        }
    }
    BOOST_CHECK( received );
}
