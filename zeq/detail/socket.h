
/* Copyright (c) 2014, Human Brain Project
 *                     Stefan.Eilemann@epfl.ch
 */


#ifndef ZEQ_DETAIL_SOCKETS_H
#define ZEQ_DETAIL_SOCKETS_H

#include <zmq.h>

namespace zeq
{
namespace detail
{

/**
 * Wrapper to hide zmq_pollitem_t from the API (it's a typedef which can't be
 * forward declared)
 */
struct Socket : public zmq_pollitem_t
{
    Socket& operator = ( const zmq_pollitem_t& from )
    {
        if( this != &from )
            *static_cast< zmq_pollitem_t* >( this ) = from;
        return *this;
    }
};

}
}

#endif
