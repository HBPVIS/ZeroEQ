
/* Copyright (c) 2015, Stefan.Eilemann@epfl.ch
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 * - Neither the name of the Human Brain Project nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

// Emits a camera which is rotating around the Y axis
// Usage: ./cameraRotator [updateIntervalInMs [rotationAngle]]

#include <zeq/zeq.h>
#include <zeq/hbp/hbp.h>
#include <servus/uri.h>

#include <cmath>
#include <thread>

int main( const int argc, char** argv )
{
    const int time = (argc > 1) ? ::atoi( argv[1] ) : 50;
    const float angle = (argc > 2) ? ::atof( argv[2] ) : 0.1f;

    zeq::Publisher publisher;
    std::vector< float > matrix( 16, 0 );
    matrix[ 0 ] = 1;
    matrix[ 5 ] = 1;
    matrix[ 9 ] = 1;
    matrix[ 15 ] = 1;

    while( publisher.publish( zeq::hbp::serializeCamera( matrix )))
    {
        const float sin = std::sin( angle );
        const float cos = std::cos( angle );

        float tmp = matrix[ 0 ];
        matrix[ 0 ]  = matrix[ 0 ] * cos - matrix[ 2 ] * sin;
        matrix[ 2 ]  = tmp * sin + matrix[ 2 ] * cos;

        tmp = matrix[ 4 ];
        matrix[ 4 ]  = matrix[ 4 ] * cos - matrix[ 6 ] * sin;
        matrix[ 6 ]  = tmp * sin + matrix[ 6 ] * cos;

        tmp = matrix[ 8 ];
        matrix[ 8 ]  = matrix[ 8 ] * cos - matrix[ 10 ] * sin;
        matrix[ 10 ] = tmp * sin + matrix[ 10 ] * cos;

        tmp = matrix[ 12 ];
        matrix[ 12 ] = matrix[ 12 ] * cos - matrix[ 14 ] * sin;
        matrix[ 14 ] = tmp * sin + matrix[ 14 ] * cos;

        std::this_thread::sleep_for( std::chrono::milliseconds( time ));
    }

    return EXIT_SUCCESS;
}
