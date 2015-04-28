
/* Copyright (c) 2015, Human Brain Project
 *                     Stefan.Eilemann@epfl.ch
 *                     Juan Hernando <jhernando@fi.upm.es>
 */


#ifndef ZEQ_DETAIL_PORT_H
#define ZEQ_DETAIL_PORT_H

#include <zeq/types.h>

namespace zeq
{
namespace detail
{

/** @return a fixed, pseudo-random port for the given name. */
uint16_t getPort( const std::string& name );

/** @return a random port number. */
uint16_t getRandomPort();

}
}

#endif
