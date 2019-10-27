#include <cstdlib>
#include <cstring>
#include <iostream>

#include "helpers.hh"
#include "fft.hh"
#include "audio.hh"
#include "wav.hh"

using namespace std;

void program_body( const string & filename )
{
    RealSignal time( 48000 * 40 );
    ComplexSignal frequency( time.size() / 2 + 1 );

    FFTPair fft { time, frequency };
    fft.time2frequency( time, frequency );

    SoundCard sound_card { "default", "default" };

    WavWrapper wav { filename };

    if ( wav.samples().size() % sound_card.period_size() ) {
	const size_t new_size = sound_card.period_size() * (1 + (wav.samples().size() / sound_card.period_size()));
	cerr << "Note: WAV length of " << wav.samples().size() << " is not multiple of "
	     << sound_card.period_size() << "; resizing to " << new_size << " samples.\n";
	wav.samples().resize( new_size );
    }

    vector<float> input( wav.samples().size() );

    sound_card.start();

    sound_card.play_and_record( wav.samples(), input );
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
