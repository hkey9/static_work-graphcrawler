#include <iostream>
#include <vector>
#include <cstdlib>
#include <chrono>
#include <omp.h>
#include "omp_tasking.hpp"

void generateMergeSortData(std::vector<int>& arr, size_t n) {
  for (size_t i = 0; i < n; ++i) arr[i] = rand();
}

void checkMergeSortResult(const std::vector<int>& arr, size_t n) {
  bool ok = true;
  for (size_t i = 1; i < n; ++i)
    if (arr[i] < arr[i - 1]) { ok = false; break; }
  if (!ok) std::cerr << "notok\n";
}

void merge_ranges(int* arr, size_t l, size_t mid, size_t r, int* temp) {
  size_t left_n = mid - l + 1;
  for (size_t i = 0; i < left_n; ++i)
    temp[i] = arr[l + i];

  size_t i = 0;
  size_t j = mid + 1;
  size_t k = l;

  while (i < left_n && j <= r) {
    if (temp[i] <= arr[j]) arr[k++] = temp[i++];
    else arr[k++] = arr[j++];
  }
  while (i < left_n) arr[k++] = temp[i++];
}

void mergesort_seq(int* arr, size_t l, size_t r, int* temp) {
  if (l >= r) return;
  size_t mid = (l + r) / 2;
  mergesort_seq(arr, l, mid, temp);
  mergesort_seq(arr, mid + 1, r, temp);
  merge_ranges(arr, l, mid, r, temp);
}

void mergesort_par_task(int* arr, size_t l, size_t r, int* temp, size_t threshold) {
  if (l >= r) return;
  size_t len = r - l + 1;
  if (len <= threshold) {
    mergesort_seq(arr, l, r, temp);
    return;
  }

  size_t mid = (l + r) / 2;

  omp_tasking::taskstart([=, &arr, &threshold]() {
    std::vector<int> temp_left(mid - l + 1);
    mergesort_par_task(arr, l, mid, temp_left.data(), threshold);
  });

  omp_tasking::taskstart([=, &arr, &threshold]() {
    std::vector<int> temp_right(r - mid);
    mergesort_par_task(arr, mid + 1, r, temp_right.data(), threshold);
  });

  omp_tasking::taskwait();

  // Use a local temp for merging
  std::vector<int> temp_local(mid - l + 1);
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

  std::vector<int> temp(n);

  auto start = std::chrono::high_resolution_clock::now();

  omp_tasking::doinparallel(threads, [&]() {
    mergesort_par_task(arr.data(), 0, (n == 0 ? 0 : n - 1), temp.data(), threshold);
  });

  auto end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> elapsed = end - start;

  std::cerr << elapsed.count() << std::endl;
  checkMergeSortResult(arr, n);

  return 0;
}