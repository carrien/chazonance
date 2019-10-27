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

SoundCard::PCM SoundCard::open_pcm( const string & name, const string & annotation, const snd_pcm_stream_t stream )
{
    snd_pcm_t * pcm = nullptr;
    alsa_check( "snd_pcm_open(" + name + ")", snd_pcm_open( &pcm, name.c_str(), stream, SND_PCM_NONBLOCK ) );
    if ( not pcm ) {
	throw runtime_error( "snd_pcm_open() returned nullptr without error" );
    }
    return PCM{ pcm, annotation + "[" + name + "]" };
}

void SoundCard::check_state( const snd_pcm_state_t state )
{
    const auto mic_state = snd_pcm_state( microphone_.first.get() );
    const auto speaker_state = snd_pcm_state( speaker_.first.get() );
    if ( (mic_state != state) or (speaker_state != state) ) {
	throw runtime_error( "expected state "s + snd_pcm_state_name( state )
			     + ", but microphone is in state " + snd_pcm_state_name( mic_state )
			     + " and speaker is in state " + snd_pcm_state_name( speaker_state ) );
    }
}

void SoundCard::set_params( PCM & pcm )
{
    alsa_check( "snd_pcm_set_params(" + pcm.second + ")",
		snd_pcm_set_params( pcm.first.get(),
				    SND_PCM_FORMAT_FLOAT_LE,
				    SND_PCM_ACCESS_RW_INTERLEAVED,
				    1,
				    48000,
				    0,
				    10667 ) );

    snd_pcm_sw_params_t *params;
    snd_pcm_sw_params_alloca( &params );
    alsa_check( "snd_pcm_sw_params_current(" + pcm.second + ")",
		snd_pcm_sw_params_current( pcm.first.get(), params ) );
    alsa_check( "snd_pcm_sw_params_set_start_threshold(" + pcm.second + ")",
		snd_pcm_sw_params_set_start_threshold( pcm.first.get(),
						       params,
						       std::numeric_limits<int32_t>::max() ) );
    alsa_check( "snd_pcm_sw_params(" + pcm.second + ")",
		snd_pcm_sw_params( pcm.first.get(), params ) );    
}

SoundCard::SoundCard( const string & microphone_name,
		      const string & speaker_name )
    : microphone_( open_pcm( microphone_name.c_str(), "microphone", SND_PCM_STREAM_CAPTURE ) ),
      speaker_( open_pcm( speaker_name.c_str(), "speaker", SND_PCM_STREAM_PLAYBACK ) ),
      linked_( false )
{
    check_state( SND_PCM_STATE_OPEN );

    set_params( microphone_ );
    set_params( speaker_ );

    check_state( SND_PCM_STATE_PREPARED );

    try {
	alsa_check( "snd_pcm_link", snd_pcm_link( microphone_.first.get(), speaker_.first.get() ) );
	linked_ = true;
    } catch ( const exception & e ) {
	cerr << "Note: cannot link clocks of given input and output devices (" << e.what() << ")\n";
    }

    check_state( SND_PCM_STATE_PREPARED );
}

void SoundCard::start() {
    vector<float> samples( 1024 );
    alsa_check( "snd_pcm_writei",
		snd_pcm_writei( speaker_.first.get(), samples.data(), samples.size() ) );

    check_state( SND_PCM_STATE_PREPARED );

    alsa_check( "snd_pcm_start(microphone)", snd_pcm_start( microphone_.first.get() ) );

    if ( not linked_ ) {
	alsa_check( "snd_pcm_start(speaker)", snd_pcm_start( speaker_.first.get() ) );
    }

    check_state( SND_PCM_STATE_RUNNING );
}

void SoundCard::pcm_closer::operator()( snd_pcm_t * const x ) const {
    alsa_check( "snd_pcm_close", snd_pcm_close( x ) );
}
