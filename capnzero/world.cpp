/**
 * Copyright (c) BBP/EPFL 2005-2014
 *                        Stefan.Eilemann@epfl.ch
 * All rights reserved. Do not distribute without further notice.
 */

#include <capnzero/world.h>
#include <viseda/version.h>

#include <iostream>

namespace capnzero
{
void World::greet()
{
    std::cout << "Capnzero world version " << viseda::Version::getRevString()
              << std::endl;
}

int World::getN( const int n )
{
    /// \todo Try harder
    /// \bug Only works for integers
    return n;
}

}
