
/* Copyright (c) 2014, Human Brain Project
 *                     Daniel Nachbaur <daniel.nachbaur@epfl.ch>
 */

#ifndef ZEQ_TYPES_H
#define ZEQ_TYPES_H

#include <lunchbox/types.h>

#include <boost/function/function1.hpp>

namespace zeq
{

class Broker;
class Event;

typedef boost::function< void( const Event& ) > EventFunc;

}

#endif
