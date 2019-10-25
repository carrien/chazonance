#include <complex>

#include "helpers.hh"

#include <fftw3.h> // needs to come after #include <complex>

template <typename T>
using AlignedSignal = std::vector<T, boost::alignment::aligned_allocator<T, 64>>;

using RealSignal = AlignedSignal<float>;
using ComplexSignal = AlignedSignal<std::complex<float>>;

struct FFTPlan
{
    fftwf_plan plan{};

    FFTPlan() {}
    FFTPlan( const std::string & what, const fftwf_plan p );
    ~FFTPlan();
    operator fftwf_plan () { return plan; }

    /* can't copy or assign */
    FFTPlan( const FFTPlan & other ) = delete;
    FFTPlan & operator=( const FFTPlan & other ) = delete;

    /* moving is okay */
    FFTPlan( FFTPlan && other );
    FFTPlan & operator=( FFTPlan && other );
};

class FFTPair
{
    size_t N;
    FFTPlan forward, backward;

    void check( RealSignal & time, ComplexSignal & frequency ) const;

public:
    FFTPair( RealSignal & time, ComplexSignal & frequency);

    void time2frequency( RealSignal & time, ComplexSignal & frequency );
    void frequency2time( ComplexSignal & frequency, RealSignal & time );

    FFTPair( const FFTPair & other ) = delete;
    FFTPair & operator=( const FFTPair & other ) = delete;
};
