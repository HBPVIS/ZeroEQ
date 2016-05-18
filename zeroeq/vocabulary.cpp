
/* Copyright (c) 2014-2016, Human Brain Project
 *                          Juan Hernando <jhernando@fi.upm.es>
 *                          Daniel Nachbaur <daniel.nachbaur@epfl.ch>
 */

#include "vocabulary.h"
#include "log.h"
#include "detail/vocabulary.h"
#include "fbevent.h"

namespace zeroeq
{
namespace vocabulary
{

FBEvent serializeEcho( const std::string& message )
{
    return detail::serializeEcho( message );
}

std::string deserializeEcho( const FBEvent& event )
{
    return detail::deserializeEcho( event );
}

}
}
