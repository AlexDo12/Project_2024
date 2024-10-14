#include <iostream>
#include "sample.h"
#include <algorithm>
#include <numeric>
#include "helper.h"

#include "mpi.h"

void sample(vector<int>& data, int taskid, int numtasks) {
    // int taskid, numtasks;
    // MPI_Comm_rank(MPI_COMM_WORLD, &taskid);
    // MPI_Comm_size(MPI_COMM_WORLD, &numtasks);

    int n;
    if (taskid == 0) {
        n = data.size();  // Total number of elements
    }
    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
    int m = numtasks;
    int chunk_size = n / m;
    vector<int> chunk(chunk_size);

    // Scatter data among workers
    MPI_Scatter(data.data(), chunk_size, MPI_INT, chunk.data(), chunk_size, MPI_INT, 0, MPI_COMM_WORLD);

    // Sort the local chunk (using quicksort)
    std::sort(chunk.begin(), chunk.end());

    // Gather samples from each process to the master
    vector<int> selected_samples(m);
    for (int i = 0; i < m; ++i) {
        selected_samples[i] = chunk[i * chunk.size() / m];
    }

    vector<int> all_samples(m * m);
    MPI_Gather(selected_samples.data(), m, MPI_INT, all_samples.data(), m, MPI_INT, 0, MPI_COMM_WORLD);

    // Master selects splitters and broadcasts them
    vector<int> splitters(m - 1);
    if (taskid == 0) {
        std::sort(all_samples.begin(), all_samples.end());
        for (int i = 1; i < m; ++i) {
            splitters[i - 1] = all_samples[i * m];
        }
    }
    MPI_Bcast(splitters.data(), m - 1, MPI_INT, 0, MPI_COMM_WORLD);

    // Split the local chunk into buckets based on splitters
    vector<vector<int>> buckets(m);
    for (int element : chunk) {
        int i = std::upper_bound(splitters.begin(), splitters.end(), element) - splitters.begin();
        buckets[i].push_back(element);
    }

    // Gather sorted buckets from all processes
    vector<int> send_counts(m), recv_counts(m);
    for (int i = 0; i < m; ++i) {
        send_counts[i] = buckets[i].size();
    }
    MPI_Alltoall(send_counts.data(), 1, MPI_INT, recv_counts.data(), 1, MPI_INT, MPI_COMM_WORLD);

    int total_recv = 0;
    for (int i = 0; i < recv_counts.size(); ++i) {
        total_recv += recv_counts[i];
    }
    vector<int> recv_data(total_recv);

    vector<int> send_displs(m), recv_displs(m);
    send_displs[0] = 0;
    recv_displs[0] = 0;
    for (int i = 1; i < m; ++i) {
        send_displs[i] = send_displs[i - 1] + send_counts[i - 1];
        recv_displs[i] = recv_displs[i - 1] + recv_counts[i - 1];
    }

    vector<int> send_data;
    for (int i = 0; i < m; ++i) {
        send_data.insert(send_data.end(), buckets[i].begin(), buckets[i].end());
    }

    MPI_Alltoallv(send_data.data(), send_counts.data(), send_displs.data(), MPI_INT,
                  recv_data.data(), recv_counts.data(), recv_displs.data(), MPI_INT, MPI_COMM_WORLD);

    // Concatenate all received buckets and locally merge them
    std::sort(recv_data.begin(), recv_data.end());
    MPI_Gather(recv_data.data(), total_recv, MPI_INT, data.data(), chunk_size, MPI_INT, 0, MPI_COMM_WORLD);
}

void test_sample(vector<int>& sorted, vector<int>& reverse, vector<int>& scrambled, vector<int>& random) {
    int taskid, numtasks;
    MPI_Comm_rank(MPI_COMM_WORLD,&taskid);
    MPI_Comm_size(MPI_COMM_WORLD,&numtasks);

    if (taskid == MASTER) {
        printf("Length: %d\n", random.size());
        printf("==== before sort ====\n[");
        for (const auto& elm : random) {
            printf("%d, ", elm);
        }
        printf("]\n");
    }
    
    double whole_computation_time;
    if (taskid == MASTER) {
        whole_computation_time = MPI_Wtime();
    }
    // sample(sorted);
    // sample(reverse);
    // sample(scrambled);
    sample(random, taskid, numtasks);
    if (taskid == MASTER) {
        whole_computation_time = MPI_Wtime() - whole_computation_time;
        printf("Total time: %f\r\n", whole_computation_time);
    }

    if (taskid == MASTER) {
        printf("==== after sort ====\n[");
        for (const auto& elm : random) {
            printf("%d, ", elm);
        }
        printf("]\n");
    }

    // check_sorted(sorted, "Sample", "sorted", taskid);
    // check_sorted(reverse, "Sample", "reverse", taskid);
    // check_sorted(scrambled, "Sample", "scrambled", taskid);
    // check_sorted(random, "Sample", "random", taskid);


}
