Files:
 - omp_tasking.hpp      : small wrapper providing doinparallel, taskstart, taskwait
 - mergesort_par.cpp    : parallel merge sort implementation
 - mergesort_seq_nocopy.cpp : your sequential reference (unchanged)
 - Makefile

Build:
  make

This will produce binaries:
 - mergesort_seq    (from your original file)
 - mergesort_par    (parallel task-based merge sort)

Run:
  ./mergesort_par <n> [threads] [threshold]
