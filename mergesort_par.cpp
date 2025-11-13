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
  for (size_t i = 1; i < n; ++i) {
    if (arr[i] < arr[i - 1]) {
      std::cerr << "notok\n";
      return;
    }
  }
}

void merge_ranges(int* arr, size_t l, size_t mid, size_t r, int* temp) {
  size_t left_n = mid - l;
  for (size_t i = 0; i < left_n; ++i)
    temp[i] = arr[l + i];

  size_t i = 0;
  size_t j = mid;
  size_t k = l;

  while (i < left_n && j < r) {
    if (temp[i] <= arr[j]) arr[k++] = temp[i++];
    else arr[k++] = arr[j++];
  }
  while (i < left_n) arr[k++] = temp[i++];
}

void mergesort_seq(int* arr, size_t l, size_t r, int* temp) {
  if (r - l <= 1) return; // base case: 1 element
  size_t mid = (l + r) / 2;
  mergesort_seq(arr, l, mid, temp);
  mergesort_seq(arr, mid, r, temp);
  merge_ranges(arr, l, mid, r, temp_
