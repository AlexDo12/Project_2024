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

#include <caliper/cali.h>
#include <caliper/cali-manager.h>
#include <adiak.hpp>

using std::vector;
using std::string;


// ============ Code ============
int main (int argc, char *argv[]) {
    CALI_CXX_MARK_FUNCTION;

    // Create caliper ConfigManager object
    cali::ConfigManager mgr;
    mgr.start();

    // Required
    int taskid, numtasks, numworkers, array_size, input_type_num;
    std::string sort_type;
    std::string input_type;
    int rc;

    if (argc == 4) {
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
            input_type_num = 1;
        } else if (input_type == "reverse") {
            input_type_num = 2;
        } else if (input_type == "1perturbed") {
            input_type_num = 3;
        } else if (input_type == "random") {
            input_type_num = 4;
        } else {
            if (taskid == MASTER) {
                printf("Unknown input type. Try again.\n");
            }
            return 0;
        }
    }
    else {
        if (taskid == MASTER) {
            printf("\n Please provide all arguments (array size, # of procs, sort type, type of input)\n");
        }  
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

    // Outputs details on current run
    if (taskid == MASTER) {
        printf("\nSort type: %s\nScramble type: %s\nArray length: %d\nNumber of processes: %d\n\n", sort_type.c_str(), input_type.c_str(), array_size, numtasks);
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

    CALI_MARK_BEGIN("data_init_runtime");
    
    // This only generates data on the master thread, everything else is an empty vec
    vector<int> data = generate_vector(array_size, numtasks, taskid, input_type_num);

    CALI_MARK_END("data_init_runtime");

    // Metadata
    adiak::init(NULL);
    adiak::launchdate();    // launch date of the job
    adiak::libraries();     // Libraries used
    adiak::cmdline();       // Command line used to launch the job
    adiak::clustername();   // Name of the cluster
    adiak::value("algorithm", sort_type); // The name of the algorithm you are using ("sample", "merge", "bitonic", "radix")
    adiak::value("programming_model", "mpi"); // e.g. "mpi"
    adiak::value("data_type", "int"); // The datatype of input elements (e.g., double, int, float)
    adiak::value("size_of_data_type", sizeof(int)); // sizeof(datatype) of input elements in bytes (e.g., 1, 2, 4)
    adiak::value("input_size", array_size); // The number of elements in input dataset (1000)
    adiak::value("input_type", input_type); // For sorting, this would be choices: ("sorted", "reverse", "random", "1perturbed")
    adiak::value("num_procs", numtasks); // The number of processors (MPI ranks)
    adiak::value("group_num", 8); // The number of your group (integer, e.g., 1, 10)
    adiak::value("implementation_source", "online"); // Where you got the source code of your algorithm. choices: ("online", "ai", "handwritten").
    
    
    if (sort_type == "merge") {
        parallel_merge_sort(data);
        adiak::value("scalability", "weak"); // The scalability of your algorithm. choices: ("strong", "weak")
    } else if (sort_type == "bitonic") {
        bitonic(data);
        adiak::value("scalability", "weak");
    } else if (sort_type == "radix") {
        testRadix(data);
    } else if (sort_type == "sample") {
        sample_sort(data, taskid, numtasks);
        adiak::value("scalability", "strong");
    } else {
        printf("Unknown sort type\n");
        MPI_Finalize();
        return 0;
    }

    // Check correctness
    CALI_MARK_BEGIN("correctness_check");
    check_sorted(data, sort_type.c_str(), input_type.c_str(), taskid);
    CALI_MARK_END("correctness_check");

    if (taskid == MASTER) {
        printf("Finished program successfully\r\n");
    }
    
    // Flush Caliper output before finalizing MPI
    mgr.stop();
    mgr.flush();
    MPI_Finalize();
        
    return 0;
}
