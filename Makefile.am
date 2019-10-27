AM_CPPFLAGS = $(CXX17_FLAGS) $(fftw3f_CFLAGS) $(alsa_CFLAGS)
AM_CXXFLAGS = $(PICKY_CXXFLAGS)

LDFLAGS = -pthread
LDADD = -lfftw3f_threads $(fftw3f_LIBS) $(alsa_LIBS) -lm

SHARED = helpers.hh fft.hh fft.cc audio.hh audio.cc

bin_PROGRAMS = demo

demo_SOURCES = demo.cc $(SHARED)
