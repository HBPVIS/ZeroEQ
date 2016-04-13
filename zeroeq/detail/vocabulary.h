
/* Copyright (c) 2014-2015, Human Brain Project
 *                          Daniel Nachbaur <daniel.nachbaur@epfl.ch>
 */

#ifndef ZEROEQ_DETAIL_VOCABULARY_H
#define ZEROEQ_DETAIL_VOCABULARY_H

#include <zeroeq/types.h>

namespace zeroeq
{
namespace vocabulary
{
namespace detail
{

Event serializeVocabulary( const EventDescriptors& vocabulary );

EventDescriptors deserializeVocabulary( const Event& event );

Event serializeRequest( const uint128_t& eventType );

uint128_t deserializeRequest( const Event& event );

zeroeq::Event serializeEcho( const std::string& msg );

std::string deserializeEcho( const zeroeq::Event& event );

zeroeq::Event serializeJSON( const uint128_t& type, const std::string& json );

std::string deserializeJSON( const zeroeq::Event& event );

void registerEvent( const uint128_t& type, const std::string& schema );

}
}
}

#endif
