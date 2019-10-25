#include <cstdlib>
#include <cstring>
#include <iostream>

#include "helpers.hh"
#include "fft.hh"

using namespace std;

void program_body( const string & filename __attribute((unused)) )
{
    RealSignal time( 48000 * 40 );
    ComplexSignal frequency( time.size() / 2 + 1 );

    FFTPair fft { time, frequency };
    fft.time2frequency( time, frequency );
}

int main( const int argc, const char * argv[] )
{
    if ( argc < 0 ) { abort(); }

    if ( argc != 2 ) {
        cerr << "Usage: " << argv[ 0 ] << " filename\n";
        return EXIT_FAILURE;
    }

    try {
        program_body( argv[ 1 ] );
    } catch ( const exception & e ) {
        cerr << e.what() << "\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
