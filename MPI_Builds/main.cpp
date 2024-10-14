// ============ Includes ============
#include <iostream>
#include <cstdlib>                    // For rand()

#include "mpi.h"
#include "helper.h"

#include "mergesort.h"
#include "radix.h"
#include "bitonic.h"
#include "sample.h"

using std::vector;


// ============ Code ============
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

    // This only generates data on the master thread, everything else is an empty vec
    vector<int> sorted_vec = generate_vector(array_size, numtasks, taskid, 1);
    vector<int> reverse_vec = generate_vector(array_size, numtasks, taskid, 2);
    vector<int> scrambled_vec = generate_vector(array_size, numtasks, taskid, 3);
    vector<int> random_vec = generate_vector(array_size, numtasks, taskid, 4);
            
    // mergesort();
    // bitonic();
    // radix();
    test_sample(sorted_vec, reverse_vec, scrambled_vec, random_vec);



    if (taskid == MASTER) {
        printf("Finished program successfully\r\n");
    }
    MPI_Finalize();













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
