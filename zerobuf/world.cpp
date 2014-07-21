/**
 * Copyright (c) BBP/EPFL 2005-2014
 *                        Stefan.Eilemann@epfl.ch
 * All rights reserved. Do not distribute without further notice.
 */

#include <zerobuf/world.h>
#include <zerobuf/version.h>

#include <iostream>

namespace zerobuf
{
void World::greet()
{
    std::cout << "Zerobuf world version " << zerobuf::Version::getRevString()
              << std::endl;
}

int World::getN( const int n )
{
    /// \todo Try harder
    /// \bug Only works for integers
    return n;
}

}
