#include <system_error>
#include <vector>
#include <iostream>

#include "audio.hh"

using namespace std;

// Error category for ALSA failures
class alsa_error_category : public error_category {
  public:
    const char *name() const noexcept override { return "alsa_error_category"; }
    string message(const int return_value) const noexcept override { return snd_strerror(return_value); }
};

class alsa_error : public std::system_error {
    std::string what_;

public:
    alsa_error( const std::string & context, const int err )
        : system_error( err, alsa_error_category() ),
          what_( context + ": " + std::system_error::what() )
    {}

    const char *what() const noexcept override { return what_.c_str(); }
};

int alsa_check( const char * context, const int return_value )
{
  if ( return_value >= 0 ) { return return_value; }
  throw alsa_error( context, return_value );
}

int alsa_check( const std::string & context, const int return_value )
{
    return alsa_check( context.c_str(), return_value );
}

string SoundCard::Device::name() const
{
    return annotation_ + "[" + device_name_ + "]";
}

SoundCard::Device::Device( const string & device_name, const string & annotation, const snd_pcm_stream_t stream )
    : device_name_( device_name ),
      annotation_( annotation ),
      pcm_( nullptr ),
      buffer_size_(), period_size_()
{
    alsa_check( "snd_pcm_open(" + name() + ")", snd_pcm_open( &pcm_, device_name_.c_str(), stream, 0 ) );
    if ( not pcm_ ) {
	throw runtime_error( "snd_pcm_open() returned nullptr without error" );
    }
}

void SoundCard::check_state( const snd_pcm_state_t state )
{
    const auto mic_state = snd_pcm_state( microphone_ );
    const auto speaker_state = snd_pcm_state( speaker_ );
    if ( (mic_state != state) or (speaker_state != state) ) {
	throw runtime_error( "expected state "s + snd_pcm_state_name( state )
			     + ", but microphone is in state " + snd_pcm_state_name( mic_state )
			     + " and speaker is in state " + snd_pcm_state_name( speaker_state ) );
    }
}

void SoundCard::set_params( Device & pcm )
{
    alsa_check( "snd_pcm_set_params(" + pcm.name() + ")",
		snd_pcm_set_params( pcm,
				    SND_PCM_FORMAT_FLOAT_LE,
				    SND_PCM_ACCESS_RW_INTERLEAVED,
				    1,
				    48000,
				    0,
				    10667 ) );

    snd_pcm_sw_params_t *params;
    snd_pcm_sw_params_alloca( &params );
    alsa_check( "snd_pcm_sw_params_current(" + pcm.name() + ")",
		snd_pcm_sw_params_current( pcm, params ) );
    alsa_check( "snd_pcm_sw_params_set_start_threshold(" + pcm.name() + ")",
		snd_pcm_sw_params_set_start_threshold( pcm,
						       params,
						       std::numeric_limits<int32_t>::max() ) );
    alsa_check( "snd_pcm_sw_params(" + pcm.name() + ")",
		snd_pcm_sw_params( pcm, params ) );    
}

SoundCard::SoundCard( const string & microphone_name,
		      const string & speaker_name )
    : microphone_( microphone_name, "microphone", SND_PCM_STREAM_CAPTURE ),
      speaker_( speaker_name, "speaker", SND_PCM_STREAM_PLAYBACK ),
      linked_( false )
{
    check_state( SND_PCM_STATE_OPEN );

    set_params( microphone_ );
    set_params( speaker_ );

    check_state( SND_PCM_STATE_PREPARED );

    try {
	alsa_check( "snd_pcm_link", snd_pcm_link( microphone_, speaker_ ) );
	linked_ = true;
    } catch ( const exception & e ) {
	cerr << "Note: cannot link clocks of these devices. Using manual sync instead. (" << e.what() << ")\n";
    }

    check_state( SND_PCM_STATE_PREPARED );
}

void SoundCard::start() {
    vector<float> samples( 1024 );
    const auto samples_written = alsa_check( "snd_pcm_writei",
					     snd_pcm_writei( speaker_, samples.data(), samples.size() ) );

    cerr << "Preroll: " << samples_written << " samples (" << samples_written / 48 << " ms).\n";

    check_state( SND_PCM_STATE_PREPARED );

    alsa_check( "snd_pcm_start(microphone)", snd_pcm_start( microphone_ ) );

    if ( not linked_ ) {
	alsa_check( "snd_pcm_start(speaker)", snd_pcm_start( speaker_ ) );
    }

    check_state( SND_PCM_STATE_RUNNING );
}

SoundCard::Device::~Device()
{
    try {
	alsa_check( "snd_pcm_close(" + name() + ")", snd_pcm_close( pcm_ ) );
    } catch ( const exception & e ) {
	cerr << "Exception in destructor: " << e.what() << endl;
    }
}
