
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
    EventDescriptor( const std::string& restName, const uint128_t& eventType,
                     const std::string& schema )
        : _restName( restName )
        , _eventType( eventType )
        , _schema( schema )
    {}
    const std::string& getRestName() const { return _restName; }
    const uint128_t& getEventType() const { return _eventType; }
    const std::string& getSchema() const { return _schema; }

private:
    const std::string _restName;
    const uint128_t _eventType;
    const std::string _schema;
};

}
}

#endif
