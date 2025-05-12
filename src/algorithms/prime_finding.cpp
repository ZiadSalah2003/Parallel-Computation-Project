#include "../../include/algorithms/prime_finding.h"
#include <vector>
#include <mpi.h>
#include <algorithm>

using namespace std;

bool isPrime(long long n) {
    if (n <= 1) 
        return false;
    if (n == 2) 
        return true;

    for (long long i = 2; i * i <= n; i++) {
        if (n % i == 0) 
            return false;
    }
    return true;
}

vector<long long> parallelPrimeFinding(long long lower_bound, long long upper_bound,
                                     int rank, int world_size, MPI_Comm comm) {
    vector<long long> local_primes;
    long long total_numbers = upper_bound - lower_bound + 1;
    
    if (total_numbers <= 0) 
        return {};

    long long chunk_size = total_numbers / world_size;
    long long remainder = total_numbers % world_size;

    long long my_start = lower_bound + rank * chunk_size + min((long long)rank, remainder);
    long long my_end = my_start + chunk_size + (rank < remainder ? 1 : 0) -1;

    for (long long i = my_start; i <= my_end; ++i) {
        if (isPrime(i)) {
            local_primes.push_back(i);
        }
    }

    int local_size = local_primes.size();
    vector<int> recv_counts;
    if (rank == 0) 
        recv_counts.resize(world_size);

    MPI_Gather(&local_size, 1, MPI_INT, (rank == 0) ? recv_counts.data() : nullptr, 1, MPI_INT, 0, comm);

    vector<long long> global_primes;
    vector<int> displs;
    if (rank == 0) {
        displs.resize(world_size);
        int total_gathered_primes = 0;
        for (int i = 0; i < world_size; ++i) {
            displs[i] = (i == 0) ? 0 : displs[i - 1] + recv_counts[i - 1];
            total_gathered_primes += recv_counts[i];
        }
        global_primes.resize(total_gathered_primes);
    }

    MPI_Gatherv(local_primes.data(), local_size, MPI_LONG_LONG,
                (rank == 0) ? global_primes.data() : nullptr,
                (rank == 0) ? recv_counts.data() : nullptr,
                (rank == 0) ? displs.data() : nullptr,
                MPI_LONG_LONG, 0, comm);

    if (rank == 0) {
        return global_primes;
    }
    return {};
}