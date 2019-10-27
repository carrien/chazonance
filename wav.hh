#ifndef WAV_HH
#define WAV_HH

#include <vector>
#include <sndfile.hh>

class WavWrapper
{
    SndfileHandle handle_;
    std::vector<float> samples_;

    const int SAMPLE_RATE = 48000;
    const int NUM_CHANNELS = 1;

public:
    WavWrapper( const std::string & filename );

    std::vector<float> & samples() { return samples_; }
};

#endif /* WAV_HH */
