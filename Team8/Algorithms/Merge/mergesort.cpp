#include <iostream>
#include "mergesort.h"
#include "mpi.h"
#include "helper.h"
#include <caliper/cali.h>
#include <caliper/cali-manager.h>
using namespace std;

// Function for parallel merge sort
void parallel_merge_sort(vector<int>& data) {
    
    int taskid, numtasks;
    MPI_Comm_rank(MPI_COMM_WORLD, &taskid);
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);

    int n;

    // Broadcast the size of the data vector to all processes
    if (taskid == 0) {
        n = data.size();
    }

    CALI_MARK_BEGIN("comm");
    CALI_MARK_BEGIN("comm_small");

    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);

    CALI_MARK_END("comm_small");
    CALI_MARK_END("comm");

    // Calculate the size of subarrays (even distribution)
    int local_size = n / numtasks;
    vector<int> local(local_size);

    CALI_MARK_BEGIN("comm");
    CALI_MARK_BEGIN("comm_large");

    // Scatter the data evenly to all processes
    MPI_Scatter(data.data(), local_size, MPI_INT, local.data(), local_size, MPI_INT, 0, MPI_COMM_WORLD);

    CALI_MARK_END("comm_large");
    CALI_MARK_END("comm");

    CALI_MARK_BEGIN("comp");
    CALI_MARK_BEGIN("comp_small");

    // Each process sorts its local subarray
    mergesort(local, 0, local_size - 1);

    CALI_MARK_END("comp_small");
    CALI_MARK_END("comp");

    CALI_MARK_BEGIN("comm");
    CALI_MARK_BEGIN("comm_large");

    // Gather sorted subarrays back at the root process
    MPI_Gather(local.data(), local_size, MPI_INT, data.data(), local_size, MPI_INT, 0, MPI_COMM_WORLD);

    CALI_MARK_END("comm_large");
    CALI_MARK_END("comm");

    CALI_MARK_BEGIN("comp");
    CALI_MARK_BEGIN("comp_large");
    
    // The root process (taskid == 0) merges the fully gathered data
    if (taskid == 0) {
        
        // Initialize merged array with the first subarray (rank 0's part)
        vector<int> merged(data.begin(), data.begin() + local_size);

        // Sequentially merge the other gathered subarrays
        for (int i = 1; i < numtasks; ++i) {
            vector<int> subarray(data.begin() + i * local_size, data.begin() + (i + 1) * local_size);
            merged = mergeArrays(merged, subarray); // Merge into the final result
        }

        data = merged; // Final sorted array
    }
    CALI_MARK_END("comp_large");
    CALI_MARK_END("comp");
}






// Merge Sort
void mergesort(vector<int>& array, int left, int right) {
    if (left < right) {
        int middle = (left + right) / 2;
        mergesort(array, left, middle);
        mergesort(array, middle + 1, right);
        merge(array, left, middle, right);
    }
}

// Merge function
void merge(vector<int>& array, int left, int middle, int right) {
    vector<int> temp(right - left + 1);
    int i = left, j = middle + 1, k = 0;

    while (i <= middle && j <= right) {
        if (array[i] <= array[j]) {
            temp[k++] = array[i++];
        } else {
            temp[k++] = array[j++];
        }
    }

    while (i <= middle) temp[k++] = array[i++];
    while (j <= right) temp[k++] = array[j++];

    for (k = 0; k < temp.size(); ++k) {
        array[left + k] = temp[k];
    }
}

// Merge Arrays function (adjusted for vectors)
vector<int> mergeArrays(const vector<int>& a, const vector<int>& b) {
    vector<int> result(a.size() + b.size());
    int i = 0, j = 0, k = 0;

    while (i < a.size() && j < b.size()) {
        if (a[i] <= b[j]) {
            result[k++] = a[i++];
        } else {
            result[k++] = b[j++];
        }
    }

    while (i < a.size()) result[k++] = a[i++];
    while (j < b.size()) result[k++] = b[j++];

    return result;
}

