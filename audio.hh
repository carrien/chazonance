#ifndef AUDIO_HH
#define AUDIO_HH

#include <memory>
#include <string>

#include <asoundlib.h>

class SoundCard {
private:
    struct pcm_closer { void operator()(snd_pcm_t * const x) const; };
    using PCM = std::pair<std::unique_ptr<snd_pcm_t, pcm_closer>, std::string>;
    PCM microphone_, speaker_;
    bool linked_;

    static PCM open_pcm( const std::string & name, const std::string & annotation, const snd_pcm_stream_t stream );
    static void set_params( PCM & pcm );

    void check_state( const snd_pcm_state_t state );

public:
    SoundCard( const std::string & microphone_name, const std::string & speaker_name );

    void start();
};

#endif /* AUDIO_HH */
