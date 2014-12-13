
/* Copyright (c) 2014, Human Brain Project
 *                     Juan Hernando <jhernando@fi.upm.es>
 */

#include "detail/serialization.h"

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

}
}
