// omp_tasking.hpp
// Minimal tasking abstraction built on OpenMP tasks.
// Provides: doinparallel(num_threads, callable), taskstart(callable), taskwait().
//
// If you already have your own omp_tasking.hpp, feel free to replace this file.

#ifndef OMP_TASKING_HPP
#define OMP_TASKING_HPP

#include <omp.h>
#include <utility>

namespace omp_tasking {

// Run `f()` inside an OpenMP parallel single region with `num_threads` threads.
template <typename F>
inline void doinparallel(int num_threads, F&& f) {
  if (num_threads <= 0) num_threads = 1;
  #pragma omp parallel num_threads(num_threads)
  {
    #pragma omp single nowait
    {
      f();
    }
  }
}

// Start a task that runs `f()`.
// Note: f should be a callable capturing required state by reference or value.
template <typename F>
inline void taskstart(F&& f) {
  #pragma omp task firstprivate(f)
  {
    f();
  }
}

// Wait for child tasks to finish.
inline void taskwait() {
  #pragma omp taskwait
  ;
}

} // namespace omp_tasking

// convenience free functions with the names requested by the assignment
using omp_tasking::doinparallel;
using omp_tasking::taskstart;
using omp_tasking::taskwait;

#endif // OMP_TASKING_HPP
