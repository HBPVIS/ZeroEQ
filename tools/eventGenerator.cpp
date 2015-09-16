
/* Copyright (c) 2015, Human Brain Project
 *                     Juan Hernando <jhernando@fi.upm.es>
 */

#include <zeq/zeq.h>
#include <zeq/hbp/hbp.h>
#include <servus/uri.h>

#include <chrono>
#include <thread>
#include <algorithm>
#include <fstream>
#include <string.h>

const char* scriptFile = 0;
std::string uri = "hbp://";

typedef std::pair< float, zeq::Event > PauseEventPair;
typedef std::vector< PauseEventPair > Events;
typedef std::vector< uint32_t > uint32_ts;

void parseArguments( int argc, char** argv );
void parseScript( const char* filename, Events& events );

int main( int argc, char** argv )
{
    parseArguments( argc, argv );

    servus::URI schema( uri );
    zeq::Publisher publisher( schema );
    Events events;
    parseScript( scriptFile, events );
    for( Events::const_iterator i = events.begin(); i != events.end(); ++i )
    {
        const float pause = i->first;
        if( pause )
        {
            std::cout << "Sleeping for " << pause << " seconds" << std::endl;
            std::this_thread::sleep_for(
                std::chrono::nanoseconds( size_t( pause * 1e9 )));
        }

        std::cout << "Sending event ";
        const zeq::Event& event = i->second;
        if( event.getType() == zeq::hbp::EVENT_TOGGLEIDREQUEST )
            std::cout << zeq::hbp::TOGGLEIDREQUEST << std::endl;
        else if( event.getType() == zeq::hbp::EVENT_CELLSETBINARYOP )
            std::cout << zeq::hbp::CELLSETBINARYOP << std::endl;
        else
            std::cout << " unknown type" << std::endl;

        publisher.publish( event );
    }

}

// Read a list of integers from the next not blank file of a stream
bool parseUint32_ts( std::istream& input, uint32_ts& numbers )
{
    input >> std::ws;
    std::string line;
    std::getline( input, line );
    if( input.fail( ))
        return false;
    numbers.clear();
    std::stringstream buffer( line );
    while( !buffer.eof( ))
    {
        uint32_t number;
        buffer >> number;
        if( buffer.fail( ))
            return false;
        numbers.push_back( number );
        buffer >> std::ws;
    }
    return true;
}

std::string trim( const std::string& trim )
{
    // Adapted from http://stackoverflow.com/a/17976541
    auto front = std::find_if_not( trim.begin(), trim.end(),
                                   []( int c ){ return std::isspace(c); } );
    auto back = std::find_if_not( trim.rbegin(), trim.rend(),
                                  []( int c ){ return std::isspace(c); } ).base();
    return back <= front ? std::string() : std::string( front, back );
}

zeq::Event parseCellSetBinaryOp( std::istream& input )
{
    uint32_ts first;
    uint32_ts second;
    zeq::hbp::CellSetBinaryOpType operation;

    if( !parseUint32_ts( input, first ))
    {
        std::cerr << "Error parsing CELLSETBINARYOP parameter 1" << std::endl;
        exit( -1 );
    }
    if( !parseUint32_ts( input, second ))
    {
        std::cerr << "Error parsing CELLSETBINARYOP parameter 2" << std::endl;
        exit( -1 );
    }
    input >> std::ws;
    std::string line;
    std::getline( input, line );
    if( input.fail( ))
    {
        std::cerr << "Error parsing CELLSETBINARYOP operation name"
                  << std::endl;
        exit( -1 );
    }
    std::string operationName = trim(line);
    if( operationName == "SYNAPTIC_PROJECTIONS" )
        operation = zeq::hbp::CELLSETOP_SYNAPTIC_PROJECTIONS;
    else
    {
        std::cerr << "Unknown operation for CELLSETBINARYOP: " << operationName
                  << std::endl;
        exit( -1 );
    }

    return zeq::hbp::serializeCellSetBinaryOp( first, second, operation );
}

zeq::Event parseToggleIDRequest( std::istream& input )
{
    uint32_ts ids;

    if( !parseUint32_ts( input, ids ))
    {
        std::cerr << "Error parsing TOGGLEIDREQUEST parameter" << std::endl;
        exit( -1 );
    }
    return zeq::hbp::serializeToggleIDRequest( ids );
}

void parseScript( std::istream& input, Events& events )
{
    input >> std::ws;
    while (!input.eof())
    {
        std::string line;
        std::getline( input, line );
        if( input.fail( ))
            break;

        std::stringstream buffer( line );

        std::string eventName;
        float pause = 0;
        // The pause parameter is optional, if the extraction fails it will
        // be zero
        buffer >> eventName >> pause;
        if( eventName == zeq::hbp::CELLSETBINARYOP )
        {
            events.push_back( std::make_pair( pause,
                                              parseCellSetBinaryOp( input )));
        }
        else if ( eventName == zeq::hbp::TOGGLEIDREQUEST )
        {
            events.push_back( std::make_pair( pause,
                                              parseToggleIDRequest( input )));
        }
        else
        {
            std::cerr << "Uknown event type: " << eventName << std::endl;
        }

        input >> std::ws;
    }

}

void parseScript( const char* filename, Events& events )
{
    if( !filename )
    {
        parseScript( std::cin, events );
        return;
    }

    std::ifstream file(filename);
    if( !file )
    {
        std::cerr << "Error opening file: " << filename << std::endl;
        exit( -2 );
    }
    parseScript( file, events );
}

void printUsageAndExit( const char* name, const int code, bool full = false )
{
    std::cerr << "Usage: " << name
              << " [--schema,-s uri] [script]" << std::endl
              << "       " << name << " --help" << std::endl;
    if (full)
    {
        std::cerr << R"(
The script file contains a list of event. Each event is specified with the
following format:
event_name pause_in_seconds
parameter_1
parameter_2
...
parameter_n

The event names are the same as the C++ enums without the EVENT_ prefix. The
pause indicates how much time the generator must wait after sending the
previous event (or startup for the first event) before sending the event.
Each of the following lines is parsed in order as a parameter of the event
type. Blank lines are ignored.

The events supported are TOGGLEIDREQUEST and CELLSETBINARYOP.

TOGGLEIDREQUEST takes one parameter which is a list of space separated integers.

CELLSETBINARYOP takes three parameters, two lists of space separated integers
and an operation name. At the moment the only operation is SYNAPTIC_PROJECTIONS.
)";
    }
    exit( code );
}

void parseArguments( int argc, char** argv )
{
    for( int i = 1; i != argc; ++i )
    {
        if( strcmp( argv[i], "--schema" ) == 0 || strcmp( argv[i],  "-s") == 0 )
        {
            if( i == argc - 1 )
            {
                std::cerr << "Missing parameter to schema" << std::endl;
                exit( -1 );
            }
            uri = argv[i + 1];
            ++i;
        }
        else if( strcmp( argv[i], "--help" ) == 0 ||
                 strcmp( argv[i], "-h" ) == 0 )
        {
            printUsageAndExit( argv[0], 0, true );
        }
        else if( scriptFile )
        {
            // This is an unexpected positional parameter
            std::cerr << "Unexpected parameter" << std::endl;
            printUsageAndExit( argv[0], -1 );
        }
        else
        {
            scriptFile = argv[i];
        }
    }
}

