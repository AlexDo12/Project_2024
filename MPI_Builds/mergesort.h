#ifndef MERGESORT_H
#define MERGESORT_H
#include <vector>

using std::vector;
#define MASTER 0

// Merge Sort declarations
void mergesort(vector<int>& data, int left, int right); // Declaration of the mergesort function
void merge(vector<int>& array, int left, int middle, int right); // Merge function for mergesort
vector<int> mergeArrays(const vector<int>& a, const vector<int>& b); // Merge arrays used in parallel sorting

// Parallel Merge Sort declarations
void parallel_merge_sort(vector<int>& data); // Declaration of the parallel merge sort function
void test_mergesort(vector<int>& vec); // Test function for merge sort

#endif
