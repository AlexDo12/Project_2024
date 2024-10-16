#include <iostream>

#include "mpi.h"
#include "helper.h"

/* ============ LOCAL FUNCTIONS ============ */
vector<int> GenerateArrayChunck(int arrayLength, int start, int order);




/* ============ CODE ============ */
// (Local func) Generates an array given the length, what value to start at, and what order to sort it by.
vector<int> GenerateArrayChunck(int arrayLength, int start, int order) {

    vector<int> chunck(arrayLength);
    
    switch(order) {
        // sorted array 
        case 1:
            for(int i = 0; i < arrayLength; i ++) {
                chunck.at(i) = start + i;
            }
        break;

        // reverse sorted
        case 2:
            for(int i = 0; i < arrayLength; i ++) {
                chunck.at(i) = arrayLength + start - i - 1;
            }
        break;

        // 1% scrambled
        case 3:
            for(int i = 0; i < arrayLength; i ++) {
                chunck.at(i) = start + i;
                if (rand() % 100 == 0) {
                    chunck.at(i) = rand();
                }
            }
        break;
        
        // random 
        case 4:
            for(int i = 0; i < arrayLength; i ++) {
                chunck.at(i) = rand();
            }
        break;
        
        // "error handling"
        default:
            printf("Error: incorrect order\n");
    }

    return chunck;
}

// Generates a vector on the master process using all processes.
vector<int> generate_vector(int length, int num_processes, int taskid, int order) {
    int local_size = (length / num_processes);
    int global_size = length;

    vector<int> local_vec;
    if (order == 2) {
        local_vec = GenerateArrayChunck(local_size, local_size*(num_processes - taskid - 1), order);
    } else {
        local_vec = GenerateArrayChunck(local_size, local_size*taskid, order);
    }

    vector<int> gathered_vec;
    if (taskid == MASTER) {
        gathered_vec.resize(length);
    }
    
    MPI_Gather(local_vec.data(), local_size, MPI_INT, gathered_vec.data(), 
        local_size, MPI_INT, MASTER, MPI_COMM_WORLD);

    // Return data if MASTER, otherwise return empty vec (don't use it, duh)
    if (taskid == MASTER) {
        return gathered_vec;
    } else {
        return vector<int>();
    }
}

// TODO: Make this parallel
// Checks if the vector is sorted
bool is_vec_sorted(vector<int> values, int taskid) {
    if (taskid == MASTER) {
        for (int i = 0; i < values.size() - 1; ++i) {
            if (values.at(i) > values.at(i + 1)) {
                return false;
            }
        }
        return true;
    }
    return false;
}

// Cleanly print out if the sort correctly sorts a certain function
void check_sorted(vector<int> data, const char* sort_type, const char* name, int taskid) {
    bool sorted = is_vec_sorted(data, taskid);
    if (taskid == MASTER) {
        if (sorted) {
            printf("%s sort correctly sorted the \"%s\" vector.\n", sort_type, name);
        } else {
            printf("%s sort DID NOT correctly sort the \"%s\" vector.\n", sort_type, name);
        }
    }
}
