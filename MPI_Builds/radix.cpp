#include <iostream>
#include <vector>
#include <cmath>
#include "mpi.h"

using std::vector;


int getBits(int num, int shift, int r) {
    return (num >> shift) & ((1 << r) - 1);
}


void radix(std::vector<int>& local_data, int total_elements, int r, int max_value, MPI_Comm comm) {
    int world_size, world_rank;
    MPI_Comm_size(comm, &world_size);
    MPI_Comm_rank(comm, &world_rank);
    
    int num_bits = std::ceil(std::log2(max_value + 1));  // Total number of bits in the max_value
    int num_blocks = (num_bits + r - 1) / r;  // Number of r-bit blocks to process
    
    std::vector<int> global_counts(1 << r);  // To store global counts for each value
    std::vector<int> local_counts(1 << r);   // To store local counts for each process

    for (int i = 0; i < num_blocks; i++) {
        int shift = i * r;  // The bit position to shift for current r-bit block

        // Step 1: Count occurrences of each r-bit value in local data
        std::fill(local_counts.begin(), local_counts.end(), 0);
        for (int num : local_data) {
            int value = getBits(num, shift, r);
            local_counts[value]++;
        }

        // Step 2: Exchange count arrays across all processes (Allreduce)
        MPI_Allreduce(local_counts.data(), global_counts.data(), 1 << r, MPI_INT, MPI_SUM, comm);

        // Step 3: Compute prefix sum (offsets) to determine final positions
        std::vector<int> offsets(1 << r, 0);
        int prefix_sum = 0;
        for (int j = 0; j < (1 << r); j++) {
            int count = global_counts[j];
            offsets[j] = prefix_sum;
            prefix_sum += count;
        }

        // Step 4: Compute processor's offset for each value based on local counts
        std::vector<int> local_offsets(1 << r, 0);
        for (int j = 0; j < (1 << r); j++) {
            local_offsets[j] = offsets[j];
            offsets[j] += local_counts[j];  // Update for redistribution
        }

        // Step 5: Redistribute elements based on calculated offsets
        std::vector<int> sorted_data(local_data.size());
        std::vector<int> send_counts(world_size, 0);
        std::vector<int> recv_counts(world_size, 0);
        std::vector<int> send_displs(world_size, 0);
        std::vector<int> recv_displs(world_size, 0);

        for (int num : local_data) {
            int value = getBits(num, shift, r);
            sorted_data[local_offsets[value]++] = num;
        }

        // Step 6: Gather sorted elements across processes (Allgather)
        MPI_Allgather(sorted_data.data(), local_data.size(), MPI_INT, local_data.data(), local_data.size(), MPI_INT, comm);
    }
}

void testRadix(vector<int>& data) {

  // split up input vector 
  int world_rank, world_size;
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);

  int chunckLength = data.size() / world_size;
  //BUG POTENTIAL: may need + 1 on end
  // TODO: replace with mpi scatter
  vector<int> currchunck(data.begin() + (world_rank*chunckLength), data.begin() + (world_rank*chunckLength + chunckLength));

  // sort array
  int maxVal = INT32_MAX;
  radix(currchunck, data.size(), 8, maxVal, MPI_COMM_WORLD);

  // verify sorted
  std::vector<int> global_data;
  if (world_rank == 0) {
    global_data.resize(data.size());  // Only allocate memory on root process
  }

  MPI_Gather(currchunck.data(), currchunck.size(), MPI_INT,
              global_data.data(), currchunck.size(), MPI_INT,
              0, MPI_COMM_WORLD);
}