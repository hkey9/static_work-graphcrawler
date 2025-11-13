CXX = g++
CXXFLAGS = -O3 -std=c++17 -fPIC -g -fopenmp
LDFLAGS = -fopenmp
LD = $(CXX)


all: mergesort_seq mergesort_par

mergesort_seq: mergesort_seq_nocopy.o
	$(LD) $(LDFLAGS) mergesort_seq_nocopy.o -o mergesort_seq

mergesort_par: mergesort_par.o
	$(LD) $(LDFLAGS) mergesort_par.o -o mergesort_par

mergesort_seq_nocopy.o: mergesort_seq_nocopy.cpp
	$(CXX) $(CXXFLAGS) -c mergesort_seq_nocopy.cpp -o mergesort_seq_nocopy.o

mergesort_par.o: mergesort_par.cpp omp_tasking.hpp
	$(CXX) $(CXXFLAGS) -c mergesort_par.cpp -o mergesort_par.o


clean:
	-rm -f *.o mergesort_par mergesort_seq

.PHONY: all clean