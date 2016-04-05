
/* Copyright (c) 2016, Human Brain Project
 *                     Stefan.Eilemann@epfl.ch
 */

#include <zeq/subscriber.h>
#include <zerobuf/data/Progress.h>
#include <chrono>
#include <iostream>
#include <map>


using std::chrono::duration_cast;
using std::chrono::high_resolution_clock;
using std::chrono::seconds;

class ProgressMonitor
{
    static const uint64_t _maxAge = 60; // seconds before kicked of the list

public:
    ProgressMonitor()
        : _progress( 0 )
        , _startTime( high_resolution_clock::now( ))
    {
        _progress.setUpdatedFunction(
            [&] {
                const uint64_t now = duration_cast< seconds >(
                    high_resolution_clock::now() - _startTime ).count();
                _operations[ _progress.getOperationString() ] =
                    { _progress.getAmount(), now };
            });
        _subscriber.subscribe( _progress );
    }

    void update()
    {
        if( _operations.empty( ))
        {
            std::cout << std::endl;
            _subscriber.receive();
        }
        else
            _subscriber.receive( _maxAge * 1000 );

        const uint64_t now = duration_cast< seconds >(
             std::chrono::high_resolution_clock::now() - _startTime ).count();

        for( auto i = _operations.begin(); i != _operations.end(); )
        {
            if( i->second.updated + _maxAge < now || i->second.amount >= 1.f )
                i = _operations.erase( i );
            else
                ++i;
        }

        if( _operations.empty( ))
            return;

        if( _operations.size() == 1 ) // print single line in place
        {
            const auto& i = *_operations.begin();
            std::cout << '\r' << i.first << ": "
                      << int( i.second.amount * 100.f ) << "% " << std::flush;
            return;
        }

        std::cout << std::endl;
        for( const auto& i : _operations )
            std::cout << i.first << ": " << int( i.second.amount * 100.f )
                      << "%" << std::endl;
    }

private:
    ProgressMonitor( const ProgressMonitor& ) = delete;
    ProgressMonitor( ProgressMonitor&& ) = delete;
    ProgressMonitor& operator = ( const ProgressMonitor& ) = delete;
    ProgressMonitor& operator = ( ProgressMonitor&& ) = delete;

    zeq::Subscriber _subscriber;
    zerobuf::data::Progress _progress;
    const high_resolution_clock::time_point _startTime;

    struct Operation
    {
        float amount;
        uint64_t updated;
    };
    std::map< std::string, Operation > _operations;
};

int main()
{
    ProgressMonitor monitor;

    while( true )
        monitor.update();
    return EXIT_SUCCESS;
}
