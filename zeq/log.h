
/* Copyright (c) 2014-2015, Human Brain Project
 *                          Juan Hernando <jhernando@fi.upm.es>
 */

#ifndef ZEQ_LOG_H
#define ZEQ_LOG_H

#include <iostream>
#define ZEQERROR std::cerr
#define ZEQWARN std::cerr
#ifdef NDEBUG
#  define ZEQINFO if( false ) std::cout
#  define ZEQLOG if( false ) std::cout
#else
#  define ZEQINFO std::cerr
#  define ZEQLOG std::cerr
#endif
#define ZEQDONTCALL \
    { ZEQERROR << "Code is not supposed to be called in this context"    \
               << std::endl; }

#define ZEQTHROW(exc) \
    {                                                               \
        ZEQINFO << exc.what() << std::endl;                         \
        throw exc;                                                  \
    }

#endif
