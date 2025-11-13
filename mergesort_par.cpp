#include <iostream>
#include <vector>
#include <cstdlib>
#include <chrono>
#include <omp.h>
#include "omp_tasking.hpp"

// Deterministic seed to compare runs reliably
void generateMergeSortData(std::vector<int>& arr, size_t n) {
  srand(1);
  for (size_t i = 0; i < n; ++i) arr[i] = rand();
}

// Enhanced checker: prints first disorder index and a small neighborhood
void checkMergeSortResult(const std::vector<int>& arr, size_t n) {
  for (size_t i = 1; i < n; ++i) {
    if (arr[i] < arr[i - 1]) {
      std::cerr << "notok\n";
      size_t start = (i >= 5) ? i - 5 : 0;
      size_t end = ((i + 5) < n) ? (i + 5) : (n - 1);
      std::cerr << "first bad index: " << i << "\n";
      std::cerr << "neighborhood [" << start << ".." << end << "]:\n";
      for (size_t k = start; k <= end; ++k) {
        std::cerr << k << ":" << arr[k] << (k==i ? " <--" : "") << "\n";
      }
      return;
    }
  }
  // ok: print nothing except success (no "notok")
}

void merge_ranges(int* arr, size_t l, size_t mid, size_t r, int* temp) {
    size_t left_n = mid - l;  // left half = [l, mid-1]
    for (size_t i = 0; i < left_n; ++i)
        temp[i] = arr[l + i];

    size_t i = 0;       // index into temp (left half)
    size_t j = mid;     // index into right half
    size_t k = l;       // index into merged array

    while (i < left_n && j < r) {
        if (temp[i] <= arr[j])
            arr[k++] = temp[i++];
        else
            arr[k++] = arr[j++];
    }

    while (i < left_n)
        arr[k++] = temp[i++];
}

void mergesort_seq(int* arr, size_t l, size_t r, int* temp) {
  // sorts range [l,r)
  if (r - l <= 1) return;
  size_t mid = (l + r) / 2;
  mergesort_seq(arr, l, mid, temp);
  mergesort_seq(arr, mid, r, temp);
  merge_ranges(arr, l, mid, r, temp);
}

void mergesort_par_task(int* arr, size_t l, size_t r, size_t threshold) {
  if (r - l <= 1) return;
  size_t len = r - l;
  if (len <= threshold) {
    std::vector<int> temp_local(len);
    mergesort_seq(arr, l, r, temp_local.data());
    return;
  }

  size_t mid = (l + r) / 2;

  // left task
  omp_tasking::taskstart([=, &arr]() {
    mergesort_par_task(arr, l, mid, threshold);
  });

  // right task
  omp_tasking::taskstart([=, &arr]() {
    mergesort_par_task(arr, mid, r, threshold);
  });

  omp_tasking::taskwait();

  // merge using local temp for left half
  std::vector<int> temp_local(mid - l);
  merge_ranges(arr, l, mid, r, temp_local.data());
}

int main(int argc, char* argv[]) {
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " <n> [threads] [threshold]\n";
    return -1;
  }

  size_t n = (size_t) std::atoll(argv[1]);
  int threads = (argc >= 3) ? std::atoi(argv[2]) : omp_get_max_threads();
  size_t threshold = (argc >= 4) ? (size_t) std::atoll(argv[3]) : 1000;

  std::vector<int> arr(n);
  generateMergeSortData(arr, n);

  auto start = std::chrono::high_resolution_clock::now();

  omp_tasking::doinparallel(threads, [&]() {
    mergesort_par_task(arr.data(), 0, n, threshold);
  });

  auto end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> elapsed = end - start;

  // Print timing to stderr (keeps stdout free)
  std::cerr << elapsed.count() << std::endl;

  /

