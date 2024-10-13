// ============ Includes ============
#include <iostream>
#include <cstdlib>                    // For rand()
#include <vector>

#include "mpi.h"

#include "mergesort.h"
#include "radix.h"
#include "bitonic.h"
#include "sample.h"

using std::cout, std::cerr, std::endl;
using std::vector;



// ============ Defines ============
#define MASTER 0
#define FROM_MASTER 1          /* setting a message type */
#define FROM_WORKER 2          /* setting a message type */



// ============ Code ============

// Generates an array given the length, what value to start at, and what order to sort it by.
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
            cerr << "Error: incorrect order\n" << endl;
    }

    return chunck;
}

// The goal was for this to generate the values in parallel. However, It only worked for up to length of 2^26, not 2^28. Couldn't figure out why.
// For now, doing this single threaded is not an issue. Its 2 seconds vs 5 seconds at max size.
// Look at commit history for "checkpoint" if you want to implement this.
// vector<int> generate_values(int length, int num_processes, int taskid, int order) {
//     // vector<int> GenerateArrayChunck(length, 0, order);
// }

bool is_vec_sorted(vector<int> values) {
    for (int i = 0; i < values.size() - 1; ++i) {
        if (values.at(i) > values.at(i + 1)) {
            return false;
        }
    }
    return true;
}


int main (int argc, char *argv[]) {
    // Required
    int taskid, numtasks, numworkers, array_size;
    int rc;

    if (argc == 2) {
        array_size = atoi(argv[1]);
    }

    else {
        printf("\n Please provide the array size\n");
        return 0;
    }

    MPI_Init(&argc,&argv);
    MPI_Comm_rank(MPI_COMM_WORLD,&taskid);
    MPI_Comm_size(MPI_COMM_WORLD,&numtasks);

    if (numtasks < 2 ) {
        printf("Need at least two MPI tasks. Quitting...\n");
        MPI_Abort(MPI_COMM_WORLD, rc);
        exit(1);
        }
    numworkers = numtasks-1;

    //Create communicator for worker processes
    MPI_Comm worker_comm;
    int thread_color;
    if (taskid == MASTER) {
        thread_color = MPI_UNDEFINED;
    } else {
        thread_color = 1;
    }
    MPI_Comm_split(MPI_COMM_WORLD, thread_color, taskid, &worker_comm);

    vector<int> sorted_vec;
    vector<int> reverse_vec;
    vector<int> scrambled_vec;
    vector<int> random_vec;

    // Only generate vectors on master to save mem
    if (taskid == MASTER) {
        sorted_vec = GenerateArrayChunck(array_size, 0, 1);
        reverse_vec = GenerateArrayChunck(array_size, 0, 2);
        scrambled_vec = GenerateArrayChunck(array_size, 0, 3);
        random_vec = GenerateArrayChunck(array_size, 0, 4);

    }
            
    // mergesort();
    // bitonic();
    // radix();
    // sample();

    













    // Can uncomment this and test values if you want
    // if (taskid == MASTER) {
    //     printf("==== SORTED ====\n[");
    //     for (const auto& elm : sorted_vec) {
    //         printf("%d, ", elm);
    //     }
    //     printf("]\n");

    //     printf("==== REVERSE ====\n[");
    //     for (const auto& elm : reverse_vec) {
    //         printf("%d, ", elm);
    //     }
    //     printf("]\n");

    //     printf("==== SCRAMBLE ====\n[");
    //     for (const auto& elm : scrambled_vec) {
    //         printf("%d, ", elm);
    //     }
    //     printf("]\n");

    //     printf("==== RANDOM ====\n[");
    //     for (const auto& elm : random_vec) {
    //         printf("%d, ", elm);
    //     }
    //     printf("]\n");

    // if (is_vec_sorted(reverse_vec)) {
    //     printf("Vector is sorted\r\n");
    // } else {
    //     printf("Vector is NOT sorted\r\n");
    // }
    // }
        
    return 0;
}
