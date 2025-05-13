#include "../../include/algorithms/sample_sort.h"
#include "../../include/utils/data_utils.h"
#include <algorithm>
#include <vector>
#include <climits>
#include <mpi.h>

using namespace std;
void merge(vector<int>& arr, int left, int mid, int right) {
    int n1 = mid - left + 1;
    int n2 = right - mid;
    vector<int> L(n1), R(n2);

    for (int i = 0; i < n1; i++){
        L[i] = arr[left + i];
    }
    for (int j = 0; j < n2; j++){
        R[j] = arr[mid + 1 + j];
    }
    
    int i = 0, j = 0, k = left;
    while (i < n1 && j < n2) {
        if (L[i] <= R[j]) {
            arr[k] = L[i];
            i++;
        } else {
            arr[k] = R[j];
            j++;
        }
        k++;
    }
    while (i < n1) {
        arr[k] = L[i];
        i++;
        k++;
    }
    while (j < n2) {
        arr[k] = R[j];
        j++;
        k++;
    }
}
void mergeSort(vector<int>& arr, int left, int right) {
    if (left < right) {
        int mid = left + (right - left) / 2;
        mergeSort(arr, left, mid);
        mergeSort(arr, mid + 1, right);
        merge(arr, left, mid, right);
    }
}
void sortVector(vector<int>& arr) {
    if (arr.empty()) return;
    mergeSort(arr, 0, arr.size() - 1);
}

vector<int> parallelSampleSort(vector<int> local_data, int global_data_size,
                             int rank, int world_size, MPI_Comm comm) {
    if (global_data_size == 0) return gatherDataGatherv(local_data, 0, rank, world_size, comm);
    sortVector(local_data);

    int num_splitters_per_proc = max(1, world_size -1);
    if (world_size == 1) num_splitters_per_proc = 0;

    vector<int> local_splitters;
    if (!local_data.empty() && num_splitters_per_proc > 0) {
        for (int i = 0; i < num_splitters_per_proc; ++i) {
            local_splitters.push_back(local_data[ (i * local_data.size()) / num_splitters_per_proc ]);
        }
    }

    int local_splitter_count = local_splitters.size();
    vector<int> recv_splitter_counts;
    if (rank == 0) recv_splitter_counts.resize(world_size);

    MPI_Gather(&local_splitter_count, 1, MPI_INT,
               (rank == 0) ? recv_splitter_counts.data() : nullptr, 1, MPI_INT, 0, comm);

    vector<int> all_splitters_gathered;
    vector<int> splitter_displs;
    int total_gathered_splitters = 0;
    if (rank == 0) {
        splitter_displs.resize(world_size);
        for (int i = 0; i < world_size; ++i) {
            splitter_displs[i] = (i == 0) ? 0 : splitter_displs[i-1] + recv_splitter_counts[i-1];
            total_gathered_splitters += recv_splitter_counts[i];
        }
        all_splitters_gathered.resize(total_gathered_splitters);
    }
    MPI_Gatherv(local_splitters.data(), local_splitter_count, MPI_INT,
                (rank == 0) ? all_splitters_gathered.data() : nullptr,
                (rank == 0) ? recv_splitter_counts.data() : nullptr,
                (rank == 0) ? splitter_displs.data() : nullptr,
                MPI_INT, 0, comm);

    vector<int> global_splitters(world_size - 1);
    if (rank == 0) {
        if (total_gathered_splitters > 0) {
            sortVector(all_splitters_gathered);
            all_splitters_gathered.erase(unique(all_splitters_gathered.begin(), all_splitters_gathered.end()), all_splitters_gathered.end());
            total_gathered_splitters = all_splitters_gathered.size();

            if (total_gathered_splitters > 0) {
                 for (int i = 0; i < world_size - 1; ++i) {
                    long long  idx = ( (i + 1) * total_gathered_splitters) / world_size;
                    if (idx >= total_gathered_splitters) idx = total_gathered_splitters -1;
                    global_splitters[i] = all_splitters_gathered[idx];
                }
            } else {
                for(int i=0; i < world_size -1; ++i) global_splitters[i] = INT_MAX;
            }

        } else if (world_size > 1) {
            for(int i=0; i < world_size -1; ++i) global_splitters[i] = INT_MAX;
        }
    }

    if (world_size > 1) {
        MPI_Bcast(global_splitters.data(), world_size - 1, MPI_INT, 0, comm);
    }

    vector<vector<int>> send_buckets_alltoall(world_size);
    if (world_size == 1) {
        send_buckets_alltoall[0] = local_data;
    } else if (!local_data.empty()){
        for (int val : local_data) {
            auto it = lower_bound(global_splitters.begin(), global_splitters.end(), val);
            int bucket_index = distance(global_splitters.begin(), it);
            send_buckets_alltoall[bucket_index].push_back(val);
        }
    }

    vector<int> send_counts_atoa(world_size);
    for (int i = 0; i < world_size; ++i) send_counts_atoa[i] = send_buckets_alltoall[i].size();

    vector<int> recv_counts_atoa(world_size);
    MPI_Alltoall(send_counts_atoa.data(), 1, MPI_INT, recv_counts_atoa.data(), 1, MPI_INT, comm);

    vector<int> send_displs_atoa(world_size, 0);
    vector<int> recv_displs_atoa(world_size, 0);
    int total_send_size_atoa = 0;
    int total_recv_size_atoa = 0;

    vector<int> send_buffer_atoa;
    send_buffer_atoa.reserve(local_data.size());
    for (int i = 0; i < world_size; ++i) {
        send_displs_atoa[i] = total_send_size_atoa;
        send_buffer_atoa.insert(send_buffer_atoa.end(), send_buckets_alltoall[i].begin(), send_buckets_alltoall[i].end());
        total_send_size_atoa += send_counts_atoa[i];
    }
    for (int i = 0; i < world_size; ++i) {
        recv_displs_atoa[i] = total_recv_size_atoa;
        total_recv_size_atoa += recv_counts_atoa[i];
    }

    vector<int> recv_buffer_atoa(total_recv_size_atoa);
    MPI_Alltoallv(send_buffer_atoa.data(), send_counts_atoa.data(), send_displs_atoa.data(), MPI_INT,
                  recv_buffer_atoa.data(), recv_counts_atoa.data(), recv_displs_atoa.data(), MPI_INT,
                  comm);

    sortVector(recv_buffer_atoa);
    local_data = recv_buffer_atoa;

    return gatherDataGatherv(local_data, 0, rank, world_size, comm);
}