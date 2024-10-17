#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include "mpi.h"
#include <caliper/cali.h>
#include <caliper/cali-manager.h>

using std::vector;

bool debug = true;

void printVector(vector<int>& data) {
    for (int i = 0; i < data.size(); i++) {
        printf("%d, ", data.at(i));
    }
    printf("\n");
}

int getBits(int num, int shift, int r) {
    return (num >> shift) & ((1 << r) - 1);
}

vector<int> radix(std::vector<int>& local_data, int total_elements, int r, int max_value, MPI_Comm comm) {
    int world_size, world_rank;
    MPI_Comm_size(comm, &world_size);
    MPI_Comm_rank(comm, &world_rank);

    int num_bits = std::ceil(std::log2(max_value + 1));  // Total number of bits in the max_value
    int num_blocks = (num_bits + r - 1) / r;  // Number of r-bit blocks to process

    if (world_rank == 0 && debug) {
        printf("radix loop has %d blocks\n", num_blocks);
    }

    for (int i = 0; i < num_blocks; i++) {
        if (debug) {
            printf("step 1 on process %d\n", world_rank);
        }
        int shift = i * r;  // The bit position to shift for current r-bit block
        int num_radix_values = 1 << r;

        // Step 1: Assign elements to bins based on radix value
        std::vector<std::vector<int>> bins(num_radix_values);
        for (int num : local_data) {
            int value = (num >> shift) & ((1 << r) - 1);
            bins[value].push_back(num);
        }

        if (debug) {
            printf("step 2 on process %d\n", world_rank);
        }

        // Step 2: Prepare send counts for all-to-all communication
        std::vector<int> send_counts(world_size, 0);
        std::vector<int> send_displs(world_size, 0);
        std::vector<int> send_buffer;

        int radix_values_per_proc = (num_radix_values + world_size - 1) / world_size;

        // Flatten bins into send_buffer and compute send_counts
        for (int value = 0; value < num_radix_values; value++) {
            int dest_proc = value / radix_values_per_proc;
            if (dest_proc >= world_size) dest_proc = world_size - 1;
            send_counts[dest_proc] += bins[value].size();
        }

        // Compute send displacements
        int total_send = 0;
        for (int j = 0; j < world_size; j++) {
            send_displs[j] = total_send;
            total_send += send_counts[j];
        }

        send_buffer.resize(total_send);

        // Now fill send_buffer with elements, keeping track of positions
        std::vector<int> temp_counts(world_size, 0);
        for (int value = 0; value < num_radix_values; value++) {
            int dest_proc = value / radix_values_per_proc;
            if (dest_proc >= world_size) dest_proc = world_size - 1;
            int index = send_displs[dest_proc] + temp_counts[dest_proc];
            std::copy(bins[value].begin(), bins[value].end(), send_buffer.begin() + index);
            temp_counts[dest_proc] += bins[value].size();
        }

        if (debug) {
            printf("step 3 on process %d\n", world_rank);
        }

        // Exchange send counts to get receive counts
        std::vector<int> recv_counts(world_size, 0);
		
		CALI_MARK_BEGIN("comm");
		CALI_MARK_BEGIN("comm_large");
        MPI_Alltoall(send_counts.data(), 1, MPI_INT,
                     recv_counts.data(), 1, MPI_INT, comm);
		CALI_MARK_END("comm_large");
		CALI_MARK_END("comm");
        // Compute receive displacements
        std::vector<int> recv_displs(world_size, 0);
        int total_recv = 0;
        for (int j = 0; j < world_size; j++) {
            recv_displs[j] = total_recv;
            total_recv += recv_counts[j];
        }

        // Prepare receive buffer
        std::vector<int> recv_buffer(total_recv);

		CALI_MARK_BEGIN("comm");
		CALI_MARK_BEGIN("comm_large");
        // Step 4: Redistribute elements based on the current radix digit
        MPI_Alltoallv(send_buffer.data(), send_counts.data(), send_displs.data(), MPI_INT, recv_buffer.data(), recv_counts.data(), recv_displs.data(), MPI_INT, comm);
		CALI_MARK_END("comm_large");
		CALI_MARK_END("comm");
        // Update local_data with the received data for the next iteration
        local_data = recv_buffer;

        if (debug) {
            printf("end loop on process %d\n", world_rank);
        }
    }

    // After all radix passes, local_data is sorted locally
    // Optionally, you can sort local_data to ensure it's sorted within each process
    std::sort(local_data.begin(), local_data.end());

    return local_data;
}

void testRadix(vector<int>& data) {
    int world_rank, world_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    int data_size = 0;

    // Root process sets data_size, others receive it
    if (world_rank == 0) {
        data_size = data.size();
    }
	
	CALI_MARK_BEGIN("comm");
    CALI_MARK_BEGIN("comm_large");
    MPI_Bcast(&data_size, 1, MPI_INT, 0, MPI_COMM_WORLD);
    CALI_MARK_END("comm_large");
    CALI_MARK_END("comm");
    // Now all processes have data_size
    int chunkLength = data_size / world_size;
    int remainder = data_size % world_size;

    std::vector<int> counts(world_size);
    std::vector<int> displs(world_size);

    // Compute counts and displs on all processes
    for (int i = 0; i < world_size; i++) {
        counts[i] = chunkLength + (i < remainder ? 1 : 0);
        displs[i] = (i == 0) ? 0 : displs[i - 1] + counts[i - 1];
    }

    int local_size = counts[world_rank];
    vector<int> currchunk(local_size);

	CALI_MARK_BEGIN("comm");
    CALI_MARK_BEGIN("comm_large");
    // Only root process has valid data
    MPI_Scatterv(world_rank == 0 ? data.data() : NULL,
                 counts.data(), displs.data(), MPI_INT,
                 currchunk.data(), local_size, MPI_INT, 0, MPI_COMM_WORLD);
    CALI_MARK_END("comm_large");
    CALI_MARK_END("comm");
    if (world_rank == 0 && debug) {
        printf("entering radix\n");
    }

    // Find the maximum value across all processes
    int local_max = (local_size > 0) ? *std::max_element(currchunk.begin(), currchunk.end()) : 0;
    int global_max;
	
	CALI_MARK_BEGIN("comm");
    CALI_MARK_BEGIN("comm_large");
    MPI_Allreduce(&local_max, &global_max, 1, MPI_INT, MPI_MAX, MPI_COMM_WORLD);
    CALI_MARK_END("comm_large");
    CALI_MARK_END("comm");
	
    // Sort array using radix sort
    vector<int> output = radix(currchunk, data_size, 8, global_max, MPI_COMM_WORLD);

    if (world_rank == 0 && debug) {
        printf("\n\nradix complete ... checking output\n");
    }

    // Gather the sizes of the output vectors from all processes
    int local_output_size = output.size();

    std::vector<int> recv_counts(world_size);
	CALI_MARK_BEGIN("comm");
    CALI_MARK_BEGIN("comm_large");
    MPI_Gather(&local_output_size, 1, MPI_INT,
               world_rank == 0 ? recv_counts.data() : NULL, 1, MPI_INT, 0, MPI_COMM_WORLD);
    CALI_MARK_END("comm_large");
    CALI_MARK_END("comm");
    // Compute displacements on root process
    std::vector<int> recv_displs(world_size);
    if (world_rank == 0) {
        recv_displs[0] = 0;
        for (int i = 1; i < world_size; i++) {
            recv_displs[i] = recv_displs[i - 1] + recv_counts[i - 1];
        }
    }

    // Gather the sorted data back to the root process
    vector<int> global_data;
    if (world_rank == 0) {
        int total_size = recv_displs[world_size - 1] + recv_counts[world_size - 1];
        global_data.resize(total_size);
    }

	CALI_MARK_BEGIN("comm");
    CALI_MARK_BEGIN("comm_large");
    MPI_Gatherv(output.data(), local_output_size, MPI_INT,
                world_rank == 0 ? global_data.data() : NULL,
                world_rank == 0 ? recv_counts.data() : NULL,
                world_rank == 0 ? recv_displs.data() : NULL,
                MPI_INT, 0, MPI_COMM_WORLD);
    CALI_MARK_END("comm_large");
    CALI_MARK_END("comm");
	
    // Update data on the root process
    if (world_rank == 0) {
        data = global_data;
    }

    if (world_rank == 0 && debug) {
        printf("Sorted array:\n");
        printVector(data);
    }
}