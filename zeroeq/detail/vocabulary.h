
/* Copyright (c) 2014-2015, Human Brain Project
 *                          Daniel Nachbaur <daniel.nachbaur@epfl.ch>
 */

#ifndef ZEROEQ_DETAIL_VOCABULARY_H
#define ZEROEQ_DETAIL_VOCABULARY_H

#include <zeroeq/types.h>

namespace zeroeq
{
namespace vocabulary
{
namespace detail
{

zeroeq::Event serializeEcho( const std::string& msg );
std::string deserializeEcho( const zeroeq::Event& event );

}
}
}

#endif
