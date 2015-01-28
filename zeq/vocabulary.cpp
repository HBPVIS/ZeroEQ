
/* Copyright (c) 2014-2015, Human Brain Project
 *                          Juan Hernando <jhernando@fi.upm.es>
 *                          Daniel Nachbaur <daniel.nachbaur@epfl.ch>
 */

#include "detail/vocabulary.h"

#include "event.h"

namespace zeq
{
namespace vocabulary
{

Event serializeEcho( const std::string& message )
{
    return detail::serializeEcho( message );
}

std::string deserializeEcho( const Event& event )
{
    return detail::deserializeEcho( event );
}

Event serializeJSON( const uint128_t& type, const std::string& json )
{
    return detail::serializeJSON( type, json );
}

std::string deserializeJSON( const Event& event )
{
    return detail::deserializeJSON( event );
}

void registerEvent( const uint128_t& type, const std::string& schema )
{
    detail::registerEvent( type, schema );
}

}
}
