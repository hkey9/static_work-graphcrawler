#ifndef OMP_TASKING_HPP
#define OMP_TASKING_HPP

#include <omp.h>
#include <utility>

namespace omp_tasking {

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

template <typename F>
inline void taskstart(F&& f) {
  #pragma omp task firstprivate(f)
  {
    f();
  }
}

inline void taskwait() {
  #pragma omp taskwait
  ;
}

}

using omp_tasking::doinparallel;
using omp_tasking::taskstart;
using omp_tasking::taskwait;

#endif