
/* Copyright (c) 2015, Human Brain Project
 *                     Juan Hernando <jhernando@fi.upm.es>
 */

#include "port.h"

#include <random>

namespace zeq
{
namespace detail
{

uint16_t getPort( const std::string& name )
{
    const uint128_t& md5 = make_uint128( name );
    return 1024 + (md5.low() % (65535-1024));
}

uint16_t getRandomPort()
{
    static std::random_device device;
    static std::minstd_rand engine( device( ));
    std::uniform_int_distribution< uint16_t > generator( 1024, 65535u );
    return generator( engine );
}

}
}

