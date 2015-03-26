
/* Copyright (c) 2015, Human Brain Project
 *                     grigori.chevtchenko@epfl.ch
 */


#ifndef ZEQ_DETAIL_EVENTDESCRIPTOR_H
#define ZEQ_DETAIL_EVENTDESCRIPTOR_H

#include <zeq/types.h>
#include <boost/noncopyable.hpp>

namespace zeq
{
namespace detail
{

struct EventDescriptor : public boost::noncopyable
{
    EventDescriptor( const std::string& restName_, const uint128_t& eventType_,
                     const std::string& schema_, const EventDirection eventDirection_ )
        : restName( restName_ )
        , eventType( eventType_ )
        , schema( schema_ )
        , eventDirection( eventDirection_ )
    {}
    const std::string restName;
    const uint128_t eventType;
    const std::string schema;
    const EventDirection eventDirection;
};

}
}

#endif
