#include <cstdlib>
#include <cstring>
#include <iostream>
#include <iomanip>

#include "helpers.hh"
#include "fft.hh"
#include "audio.hh"
#include "wav.hh"
#include "constants.hh"

using namespace std;

template <typename Signal>
tuple<float, float, size_t> amplitude( const Signal & samples )
{
    if ( samples.empty() ) {
	throw runtime_error( "samples cannot be empty" );
    }

    float sum = 0;
    float peak = -1;
    size_t peak_location = -1;

    for ( size_t i = 0; i < samples.size(); i++ ) {
	const float val = norm( samples[ i ] );
	sum += val;
	if ( val > peak ) {
	    peak = val;
	    peak_location = i;
	}
    }
    return { sqrt( sum / samples.size() ), sqrt( peak ), peak_location };
}

float dB( const float x )
{
    return 20 * log10( x );
}

void program_body( const string & filename )
{
    WAV wav;
    wav.read_from( filename );

    {
	SoundCard sound_card { "default", "default" };

	if ( wav.samples().size() % sound_card.period_size() ) {
	    const size_t new_size = sound_card.period_size() * (1 + (wav.samples().size() / sound_card.period_size()));
	    cerr << "Note: WAV length of " << wav.samples().size() << " is not multiple of "
		 << sound_card.period_size() << "; resizing to " << new_size << " samples.\n";
	    wav.samples().resize( new_size );
	}
    }

    ComplexSignal frequency( wav.samples().size() / 2 + 1 );
    FFTPair fft { wav.samples(), frequency };

    RealSignal input( wav.samples().size() );

    cout << fixed << setprecision( 2 );

    for ( unsigned int iter = 0;; iter++ ) {
	/* write out */
	wav.write_to( filename + "-generation" + to_string( iter ) + ".wav" );

	/* eliminate frequencies below 20 Hz */
	fft.time2frequency( wav.samples(), frequency );
	for ( unsigned int i = 0; i < frequency.size(); i++ ) {
	    if ( i * MAX_FREQUENCY / frequency.size() < 20.0 ) {
		frequency[ i ] = 0;
	    }
	}
	fft.frequency2time( frequency, wav.samples() );

	/* find RMS amplitude and peak amplitudes */
	const auto [ rms, peak, peak_location ] = amplitude( wav.samples() );
	cout << "Iteration " << iter << ", playing " << wav.samples().size() / float( SAMPLE_RATE )
	     << " seconds with RMS amplitude = " << dB( rms ) << " dB"
	     << " and peak amplitude = " << dB( peak ) << " dB"
	     << " @ " << peak_location / float( SAMPLE_RATE ) << " s.\n";

	fft.time2frequency( wav.samples(), frequency );

	const auto [ rms_freq, peak_freq, peak_location_freq ] = amplitude( frequency );
	cout << "In frequency domain, RMS amplitude = " << dB( rms_freq ) << " dB"
	     << " and peak amplitude = " << dB( peak_freq ) << " dB"
	     << " @ " << peak_location_freq * MAX_FREQUENCY / frequency.size() << " Hz.\n";

	/* play and record */

	SoundCard sound_card { "default", "default" };
	sound_card.start();
	sound_card.play_and_record( wav.samples(), input );
	wav.samples() = input;

	cout << endl;
    }
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
