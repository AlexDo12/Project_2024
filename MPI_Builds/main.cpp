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

// generates arry chunck
// args:
// total array length length
// number of processes
// what process is calling
// array order
// 1: sorted
// 2: reverse sorted
// 3: 1% scrambled
// 4: random
// returns:
// pointer to array start
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
            //TODO: implement
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


vector<int> vectorTester(int length, int num_processes, int taskid, int order) {
    double time_taken;
    if (taskid == MASTER) {
        printf("================\r\nvector tester running\r\n");
        time_taken = MPI_Wtime();
    }

    int local_size = (length / num_processes);
    int global_size = length;

    vector<int> local_vec = GenerateArrayChunck(local_size, local_size*taskid, order);
    vector<int> gathered_vec(length);


    MPI_Gather(local_vec.data(), local_size, MPI_INT, gathered_vec.data(), 
        local_size, MPI_INT, MASTER, MPI_COMM_WORLD);


    if (taskid == MASTER) {
        // printf("==== Combined Vector ====\n");
        // printf("[");
        // for (const auto& elem : gathered_vec) {
        //     printf("%d, ", elem);
        // }
        // printf("]");


        time_taken = MPI_Wtime() - time_taken;
        printf("Time taken was %f seconds\r\n", time_taken);
    }

    return local_vec;
}


int main (int argc, char *argv[]) {
    int taskid, numtasks, numworkers;
    double time_taken;

    MPI_Init(&argc,&argv);
    MPI_Comm_rank(MPI_COMM_WORLD,&taskid);
    MPI_Comm_size(MPI_COMM_WORLD,&numtasks);

    if (numtasks < 2 ) {
        printf("Need at least two MPI tasks. Quitting...\n");
        // MPI_Abort(MPI_COMM_WORLD, rc);
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

        
    // mergesort();
    // bitonic();
    // radix();
    // sample();

    vectorTester(268435456, numtasks, taskid, 1);






    //TODO: test array checker
        
    return 0;
}
