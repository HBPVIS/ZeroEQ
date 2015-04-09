
/* Copyright (c) 2015, Human Brain Project
 *                     Stefan.Eilemann@epfl.ch
 */


#ifndef ZEQ_DETAIL_PORT_H
#define ZEQ_DETAIL_PORT_H

#include <zmq.h>

namespace zeq
{
namespace detail
{
/** @return a fixed, pseudo-random port for the given name. */
inline uint16_t getPort( const std::string& name )
{
    const lunchbox::uint128_t& md5 = lunchbox::make_uint128( name );
    return 1024 + (md5.low() % (65535-1024));
}
}
}

#endif
