#include "../../include/algorithms/radix_sort.h"
#include "../../include/utils/data_utils.h"
#include <algorithm>
#include <vector>
#include <mpi.h>

using namespace std;

int getRadixMax(const vector<int>& arr) {
    if (arr.empty()) return 0;
    return *max_element(arr.begin(), arr.end());
}

void localCountingSortForRadix(vector<int>& arr, int exp) {
    if (arr.empty()) return;
    int n = arr.size();
    vector<int> output(n);
    vector<int> count(10, 0);

    for (int i = 0; i < n; i++) count[(arr[i] / exp) % 10]++;
    for (int i = 1; i < 10; i++) count[i] += count[i - 1];
    for (int i = n - 1; i >= 0; i--) {
        output[count[(arr[i] / exp) % 10] - 1] = arr[i];
        count[(arr[i] / exp) % 10]--;
    }
    arr = output;
}

vector<int> parallelRadixSort(vector<int> local_data, int global_data_size,
                            int rank, int world_size, MPI_Comm comm) {
    int local_max = local_data.empty() ? 0 : getRadixMax(local_data);
    int global_max = 0;
    MPI_Allreduce(&local_max, &global_max, 1, MPI_INT, MPI_MAX, comm);

    if (global_max == 0) {
        return gatherDataGatherv(local_data, 0, rank, world_size, comm);
    }

    for (int exp = 1; global_max / exp > 0; exp *= 10) {
        localCountingSortForRadix(local_data, exp);
        MPI_Barrier(comm);

        vector<int> local_digit_counts(world_size, 0);
        vector<vector<int>> send_buckets(world_size);

        vector<vector<int>> digit_buckets(10);
        for (int x : local_data) {
            digit_buckets[(x / exp) % 10].push_back(x);
        }
    }

    vector<int> gathered_data = gatherDataGatherv(local_data, 0, rank, world_size, comm);
    if (rank == 0) {
        sort(gathered_data.begin(), gathered_data.end());
    }
    return gathered_data;
}