
/* Copyright (c) 2015, Human Brain Project
 *                     Daniel.Nachbaur@epfl.ch
 */

#ifndef ZEQ_URI_H
#define ZEQ_URI_H

#include <zeq/api.h>

#include <servus/uri.h> // base class

namespace zeq
{

/**
 * Enhances servus::URI to guarantee the existance of a schema and to allow
 * construction of [host][:port] URIs from string.
 */
class URI : private servus::URI
{
public:
    /** Create a default URI in the form "tcp://" */
    ZEQ_API URI();

    ZEQ_API ~URI();

    ZEQ_API URI( const URI& from );

    /** Create URI from string, set schema to "tcp" if empty */
    ZEQ_API explicit URI( const std::string& uri );

    /** Create URI from string, set schema to "tcp" if empty */
    ZEQ_API explicit URI( const char* uri );

    /** Convert from servus::URI, set schema to "tcp" if empty */
    ZEQ_API explicit URI( const servus::URI& from );

    ZEQ_API URI& operator = ( const URI& rhs );

    /* Convert from servus::URI, set schema to "tcp" if empty */
    ZEQ_API URI& operator = ( const servus::URI& rhs );

    ZEQ_API bool operator == ( const URI& rhs ) const;

    ZEQ_API bool operator == ( const servus::URI& rhs ) const;

    ZEQ_API bool operator != ( const URI& rhs ) const;

    ZEQ_API bool operator != ( const servus::URI& rhs ) const;

    /** Convert this URI to a servus::URI */
    const servus::URI& toServusURI() const { return *this; }

    /** @name servus::URI API */
    //@{
    using servus::URI::getScheme;
    using servus::URI::getHost;
    using servus::URI::getPort;
    using servus::URI::getPath;
    using servus::URI::setHost;
    using servus::URI::setPort;
    //@}
};

inline std::ostream& operator << ( std::ostream& os, const URI& uri )
{
    return os << uri.toServusURI();
}

} // namespace zeq

namespace std
{
inline std::string to_string( const zeq::URI& uri )
{
    return to_string( uri.toServusURI( ));
}
}

#endif
