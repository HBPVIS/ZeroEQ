
/* Copyright (c) 2014-2016, Human Brain Project
 *                          Juan Hernando <jhernando@fi.upm.es>
 *                          Daniel Nachbaur <daniel.nachbaur@epfl.ch>
 */

#include "vocabulary.h"

#include "detail/vocabulary.h"
#include "event.h"

namespace zeroeq
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
