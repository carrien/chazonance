#include <iostream>
#include <exception>
#include <iomanip>

#include "wav.hh"

using namespace std;

WavWrapper::WavWrapper( const string & filename )
    : handle_( filename ),
      samples_()
{
    if ( handle_.error() ) {
	throw runtime_error( filename + ": " + handle_.strError() );
    }

    if ( handle_.format() != (SF_FORMAT_WAV | SF_FORMAT_FLOAT) ) {
	throw runtime_error( filename + ": not a 32-bit float PCM WAV file" );
    }

    if ( handle_.samplerate() != SAMPLE_RATE ) {
	throw runtime_error( filename + " sample rate is " + to_string( handle_.samplerate() ) + ", not " + to_string( SAMPLE_RATE ) );
    }

    if ( handle_.channels() != NUM_CHANNELS ) {
	throw runtime_error( filename + " channel # is " + to_string( handle_.channels() ) + ", not " + to_string( NUM_CHANNELS ) );
    }

    cerr << fixed << setprecision( 2 ) << "File " << filename << " duration: "
	 << float( handle_.frames() ) / SAMPLE_RATE << " s.\n";

    samples_.resize( handle_.frames() );

    /* read file into memory */
    const auto retval = handle_.read( samples_.data(), samples_.size() );
    if ( retval != int( samples_.size() ) ) {
	throw runtime_error( "unexpected read of " + to_string( retval ) + " samples" );
    }

    /* verify EOF */
    float dummy;
    if ( 0 != handle_.read( &dummy, 1 ) ) {
	throw runtime_error( "unexpected extra data in WAV file" );
    }
}
