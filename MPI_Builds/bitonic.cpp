#include "bitonic.h"

void bitonic(std::vector<int>& random) {
    int rank, size;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int total_elements = 0;
    if (rank == MASTER) {
        total_elements = random.size();
    }

    CALI_MARK_BEGIN("comm");
    CALI_MARK_BEGIN("comm_small");
    MPI_Bcast(&total_elements, 1, MPI_INT, MASTER, MPI_COMM_WORLD);
    CALI_MARK_END("comm_small");
    CALI_MARK_END("comm");

    int elements_per_process = total_elements / size;

    std::vector<int> local_array(elements_per_process);

    CALI_MARK_BEGIN("comm");
    CALI_MARK_BEGIN("comm_large");
    MPI_Scatter(random.data(), elements_per_process, MPI_INT, local_array.data(), elements_per_process, MPI_INT, MASTER, MPI_COMM_WORLD);
    CALI_MARK_END("comm_large");
    CALI_MARK_END("comm");

    // Each process sorts its local array
    CALI_MARK_BEGIN("comp");
    CALI_MARK_BEGIN("comp_large");
    std::sort(local_array.begin(), local_array.end());
    CALI_MARK_END("comp_large");
    CALI_MARK_END("comp");

    // Outer loop for the size of the subsequences to be merged
    // Number of phases is log_2(size)
    for (int k = 2; k <= size; k *= 2) {

        // Inner loop for the distance of the exchange partner
        for (int j = k / 2; j > 0; j /= 2) {

            // Calculate the partner process with which to compare and exchange
            int partner = rank ^ j;

            // Determine the sorting direction (ascending or descending)
            bool ascending = ((rank & k) == 0);

            if (partner < size) {
                compareExchange(local_array, partner, rank, elements_per_process, ascending);
            }
            CALI_MARK_BEGIN("comm");
            CALI_MARK_BEGIN("comm_small");
            MPI_Barrier(MPI_COMM_WORLD);
            CALI_MARK_END("comm_small");
            CALI_MARK_END("comm");
        }
    }

    // Gather the sorted subarrays back to the root process
    CALI_MARK_BEGIN("comm");
    CALI_MARK_BEGIN("comm_large");
    MPI_Gather(local_array.data(), elements_per_process, MPI_INT, random.data(), elements_per_process, MPI_INT, MASTER, MPI_COMM_WORLD);
    CALI_MARK_END("comm_large");
    CALI_MARK_END("comm");

    /*
    if (rank == MASTER)
    {
        std::cout << "Sorted array: ";
        for (int i = 0; i < random.size(); ++i)
        {
            std::cout << random.at(i) << ", ";
        }
    }
    */
}

void compareExchange(std::vector<int>& local_array, int partner, int rank, int elements_per_process, bool ascending) {
    MPI_Status status;
    std::vector<int> partner_array(elements_per_process);

    // Send local_array to the partner and receive partner_array from the partner simultaneously
    CALI_MARK_BEGIN("comm");
    CALI_MARK_BEGIN("comm_large");
    MPI_Sendrecv(local_array.data(), elements_per_process, MPI_INT, partner, 0,partner_array.data(), elements_per_process, MPI_INT, partner, 0,MPI_COMM_WORLD, &status);
    CALI_MARK_END("comm_large");
    CALI_MARK_END("comm");

    // Combine the two arrays
    CALI_MARK_BEGIN("comp");
    CALI_MARK_BEGIN("comp_large");
    std::vector<int> combined_array(elements_per_process * 2);
    std::merge(local_array.begin(), local_array.end(), partner_array.begin(), partner_array.end(), combined_array.begin());
    
    // Decide which half to keep based on sorting direction and copy selected half back into process local array
    if (ascending) {
        if (rank < partner) {
            // Keep the first (lower) half for ascending order
            std::copy(combined_array.begin(), combined_array.begin() + elements_per_process, local_array.begin());
        } else {
            // Keep the second (upper) half for ascending order
            std::copy(combined_array.begin() + elements_per_process, combined_array.end(), local_array.begin());
        }
    } else {
        if (rank < partner) {
            // Keep the second (upper) half for descending order
            std::copy(combined_array.begin() + elements_per_process, combined_array.end(), local_array.begin());
        } else {
            // Keep the first (lower) half for descending order
            std::copy(combined_array.begin(), combined_array.begin() + elements_per_process, local_array.begin());
        }
    }
    CALI_MARK_END("comp_large");
    CALI_MARK_END("comp");
  
}