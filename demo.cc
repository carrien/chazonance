#include <cstdlib>
#include <cstring>
#include <iostream>

#include "helpers.hh"
#include "fft.hh"
#include "audio.hh"

using namespace std;

void program_body( const string & microphone_name, const string & speaker_name )
{
    RealSignal time( 48000 * 40 );
    ComplexSignal frequency( time.size() / 2 + 1 );

    FFTPair fft { time, frequency };
    fft.time2frequency( time, frequency );

    SoundCard sound_card { microphone_name, speaker_name };
    sound_card.start();
}

int main( const int argc, const char * argv[] )
{
    if ( argc < 0 ) { abort(); }

    if ( argc != 3 ) {
        cerr << "Usage: " << argv[ 0 ] << " MICROPHONE SPEAKER\n";
	cerr << "       [Hint: use `aplay --list-pcms` to list device names]\n";
        return EXIT_FAILURE;
    }

    try {
        program_body( argv[ 1 ], argv[ 2 ] );
    } catch ( const exception & e ) {
        cerr << e.what() << "\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
