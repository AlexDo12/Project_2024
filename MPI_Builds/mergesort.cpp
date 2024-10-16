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
    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // Calculate the fixed chunk size
    int local_size = n / numtasks;
    vector<int> local(local_size);

    // Scatter the data evenly to all processes
    MPI_Scatter(data.data(), local_size, MPI_INT, local.data(), local_size, MPI_INT, 0, MPI_COMM_WORLD);

    // Each process sorts its local subarray
    mergesort(local, 0, local_size - 1);

    // Begin recursive pairwise merging
    int step = 1;
    while (step < numtasks) {
        if (taskid % (2 * step) == 0) {
            // Receive data from taskid + step
            if (taskid + step < numtasks) {
                vector<int> partner_data(local_size);
                MPI_Recv(partner_data.data(), local_size, MPI_INT, taskid + step, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                
                // Merge local data with received data
                local = mergeArrays(local, partner_data);
            }
        } else {
            // Send local data to taskid - step
            int destination = taskid - step;
            MPI_Send(local.data(), local_size, MPI_INT, destination, 0, MPI_COMM_WORLD);
            break;
        }
        step *= 2;
    }

    // The master process (taskid == 0) now holds the fully merged array
    if (taskid == 0) {
        data = local;
    }
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

// Test function that tests the merge sort for different arrangements of numbers.
void test_mergesort(vector<int>& vec) {
    int taskid, numtasks;
    MPI_Comm_rank(MPI_COMM_WORLD, &taskid);
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);

    double whole_computation_time;
    if (taskid == 0) {
        whole_computation_time = MPI_Wtime();
    }

    // Apply the parallel_merge_sort function for each arrangement
    parallel_merge_sort(vec);
 

    if (taskid == 0) {
        whole_computation_time = MPI_Wtime() - whole_computation_time;
        printf("Total time: %f\r\n", whole_computation_time);
    }

    // Check if arrays are sorted after merge sort
    check_sorted(vec, "MergeSort", "hi", taskid);
 
}
