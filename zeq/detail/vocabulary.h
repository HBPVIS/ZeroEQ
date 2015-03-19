
/* Copyright (c) 2014-2015, Human Brain Project
 *                          Daniel Nachbaur <daniel.nachbaur@epfl.ch>
 */

#ifndef ZEQ_DETAIL_VOCABULARY_H
#define ZEQ_DETAIL_VOCABULARY_H

#include <zeq/types.h>

namespace zeq
{
namespace vocabulary
{
namespace detail
{

Event serializeVocabulary( const EventDescriptors& vocabulary );

EventDescriptors deserializeVocabulary( const Event& event );

zeq::Event serializeEcho( const std::string& msg );

std::string deserializeEcho( const zeq::Event& event );

zeq::Event serializeJSON( const uint128_t& type, const std::string& json );

std::string deserializeJSON( const zeq::Event& event );

void registerEvent( const uint128_t& type, const std::string& schema );

const std::string& getSchema( const uint128_t& type );

}
}
}

#endif
