#ifndef WAV_HH
#define WAV_HH

#include <vector>
#include <sndfile.hh>

#include "helpers.hh"

class WavWrapper
{
    SndfileHandle handle_;
    RealSignal samples_;

    const int SAMPLE_RATE = 48000;
    const int NUM_CHANNELS = 1;

public:
    WavWrapper( const std::string & filename );

    RealSignal & samples() { return samples_; }
};

#endif /* WAV_HH */
