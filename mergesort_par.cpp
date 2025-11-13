// mergesort_par.cpp
// Parallel merge sort using omp_tasking.hpp (doinparallel, taskstart, taskwait).
// Usage: ./mergesort_par <n> [threads] [threshold]
//  - n: number of elements to sort
//  - threads: (optional) number of threads for doinparallel (default: omp_get_max_threads())
//  - threshold: (optional) size threshold below which sorting is done sequentially (default: 1000)

#include <iostream>
#include <vector>
#include <cstdlib>
#include <chrono>
#include <omp.h>
#include "omp_tasking.hpp"

void generateMergeSortData (std::vector<int>& arr, size_t n) {
  for (size_t i = 0; i < n; ++i) arr[i] = rand();
}

void checkMergeSortResult (const std::vector<int>& arr, size_t n) {
  bool ok = true;
  for (size_t i = 1; i < n; ++i)
    if (arr[i] < arr[i-1]) { ok = false; break; }
  if (!ok) std::cerr << "notok\n";
}

// merge [l..mid-1] and [mid..r] (inclusive ranges)
void merge_ranges(int* arr, size_t l, size_t mid, size_t r, int* temp) {
  size_t left_n = mid - l;
  // copy left half to temp
  for (size_t i = 0; i < left_n; ++i) temp[i] = arr[l + i];

  size_t i = 0;        // index in temp (left)
  size_t j = mid;      // index in right half (arr)
  size_t k = l;        // index to write

  while (i < left_n && j <= r) {
    if (temp[i] <= arr[j]) arr[k++] = temp[i++];
    else arr[k++] = arr[j++];
  }
  while (i < left_n) arr[k++] = temp[i++];
}

// sequential mergesort (operates in-place using temp buffer)
void mergesort_seq(int* arr, size_t l, size_t r, int* temp) {
  if (l >= r) return;
  if (r - l == 1) {
    if (arr[l] > arr[r]) {
      int t = arr[l];
      arr[l] = arr[r];
      arr[r] = t;
    }
    return;
  }
  size_t mid = (l + r) / 2;
  mergesort_seq(arr, l, mid, temp);
  mergesort_seq(arr, mid + 1, r, temp);
  merge_ranges(arr, l, mid + 1, r, temp);
}

// parallel mergesort using tasking API and threshold to avoid small tasks
void mergesort_par_task(int* arr, size_t l, size_t r, int* temp, size_t threshold) {
  if (l >= r) return;
  size_t len = r - l + 1;
  if (len <= threshold) {
    mergesort_seq(arr, l, r, temp);
    return;
  }

  size_t mid = (l + r) / 2;
  // spawn left and right as tasks
  omp_tasking::taskstart([=,&arr,&temp,&threshold]() {
    mergesort_par_task(arr, l, mid, temp, threshold);
  });
  omp_tasking::taskstart([=,&arr,&temp,&threshold]() {
    mergesort_par_task(arr, mid + 1, r, temp, threshold);
  });

  // wait for both to finish before merging
  omp_tasking::taskwait();

  // merge halves
  merge_ranges(arr, l, mid + 1, r, temp);
}

int main (int argc, char* argv[]) {
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " <n> [threads] [threshold]\n";
    return -1;
  }

  size_t n = (size_t) std::atoll(argv[1]);
  int threads = (argc >= 3) ? std::atoi(argv[2]) : omp_get_max_threads();
  size_t threshold = (argc >= 4) ? (size_t) std::atoll(argv[3]) : 1000;

  std::vector<int> arr(n);
  generateMergeSortData(arr, n);

  std::vector<int> temp(n);

  auto start = std::chrono::high_resolution_clock::now();

  // create the parallel region and run a single top-level task to begin recursion
  omp_tasking::doinparallel(threads, [&]() {
    // top-level call: we'll internally spawn tasks for left/right
    mergesort_par_task(&(arr[0]), 0, (n==0?0:n-1), &(temp[0]), threshold);
  });

  auto end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> elapsed = end - start;

  std::cerr << elapsed.count() << std::endl;
  checkMergeSortResult(arr, n);

  return 0;
}