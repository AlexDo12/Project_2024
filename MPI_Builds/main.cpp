// ============ Includes ============
#include <iostream>
#include <cstdlib>                    // For rand()
#include <string>

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
    int taskid, numtasks, numworkers, array_size, input_type_num;
    string sort_type;
    string input_type;
    int rc;

    if (argc < 5) {
        array_size = atoi(argv[1]);
        sort_type = argv[2];
        input_type = argv[3];

        // Verify array size is a power of two to prevent algorithms breaking and needless credit usage
        if ((array_size == 0) || (array_size & (array_size - 1))) {
            printf("Array size must be a power of two, not %d", array_size);
            return 1;
        }

        // Process input array
        if (input_type == "sorted") {
            printf("sorted\n");
            input_type_num = 1;
        } else if (input_type == "reverse") {
            printf("reverse\n");
            input_type_num = 2;
        } else if (input_type == "1perturbed") {
            printf("1 perturbed\n");
            input_type_num = 3;
        } else if (input_type == "random") {
            printf("random\n");
            input_type_num = 4;
        } else {
            printf("Unknown input type. Try again.\n");
            return 0;
        }
        
    }

    else {
        printf("\n Please provide all arguments (array size, # of procs, sort type, type of input)\n");
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

    // Verify number of processes are a power of two to prevent algorithms breaking and needless credit usage
    if ((numtasks == 0) || (numtasks & (numtasks - 1))) {
        if (taskid == MASTER) {
            printf("Number of processes must be a power of two, not %d", numtasks);
        }
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    //Create communicator for worker processes
    MPI_Comm worker_comm;
    int thread_color;
    if (taskid == MASTER) {
        thread_color = MPI_UNDEFINED;
    } else {
        thread_color = 1;
    }
    MPI_Comm_split(MPI_COMM_WORLD, thread_color, taskid, &worker_comm);

    if (taskid == MASTER) {
        printf("Num of proc: %d\n", numtasks);
        // for (int i = 0; i < argc; i++) {
        //     printf("Argument %d: %s\n", i, argv[i]);
        // }
        printf("Sort type: %s\n", sort_type);
        printf("Input type: %s\n", input_type);

    }

    // This only generates data on the master thread, everything else is an empty vec
    vector<int> data = generate_vector(array_size, numtasks, taskid, input_type_num);
            
    if (sort_type == "merge") {
        printf("running merge\n");
        // mergesort();
    } else if (sort_type == "bitonic") {
        printf("running bitonic\n");
        // bitonic();
    } else if (sort_type == "radix") {
        printf("running radix\n");
        // radix();
    } else if (sort_type == "sample") {
        printf("running sample\n");
        // test_sample(data);
    } else {
        printf("Unknown sort type.");
    }

    if (taskid == MASTER) {
        printf("Input type: %d\n", input_type_num);
    }



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
