#include "../../include/algorithms/bitonic_sort.h"
#include "../../include/utils/data_utils.h"
#include <algorithm>
#include <vector>
#include <mpi.h>
#include <iostream>

using namespace std;

void compareExchange(vector<int>& arr, int i, int j, bool ascending) {
    if ((arr[i] > arr[j] && ascending) || (arr[i] < arr[j] && !ascending)) {
        swap(arr[i], arr[j]);
    }
}

void bitonicMergeLocal(vector<int>& arr, int start, int len, bool ascending) {
    if (len > 1) {
        int k = len / 2;
        for (int i = start; i < start + k; i++) {
            compareExchange(arr, i, i + k, ascending);
        }
        bitonicMergeLocal(arr, start, k, ascending);
        bitonicMergeLocal(arr, start + k, k, ascending);
    }
}

void bitonicSortLocalRecursive(vector<int>& arr, int start, int len, bool ascending) {
    if (len > 1) {
        int k = len / 2;
        bitonicSortLocalRecursive(arr, start, k, true);       
        bitonicSortLocalRecursive(arr, start + k, k, false);  
        bitonicMergeLocal(arr, start, len, ascending); 
    }
}

vector<int> parallelBitonicSort(vector<int> local_data, int global_data_size_orig,
                              int rank, int world_size, MPI_Comm comm) {
    
    sort(local_data.begin(), local_data.end());

    for (int k = 2; k <= world_size; k *= 2) { 
        for (int j = k / 2; j > 0; j /= 2) {   
            int partner_rank = rank ^ j; 

            if (partner_rank < world_size && ( (rank % k) < (k/2) ? (partner_rank == rank + j) : (partner_rank == rank -j) ) ) {
        
                vector<int> partner_data(local_data.size());
                MPI_Sendrecv(local_data.data(), local_data.size(), MPI_INT, partner_rank, 0,
                          partner_data.data(), partner_data.size(), MPI_INT, partner_rank, 0,
                          comm, MPI_STATUS_IGNORE);
                
                bool ascending_merge = ((rank / k) % 2 == 0);

                vector<int> merged_data;
                merged_data.reserve(local_data.size() + partner_data.size());
                merge(local_data.begin(), local_data.end(),
                    partner_data.begin(), partner_data.end(),
                    back_inserter(merged_data)); 

                if (ascending_merge) {
                    if (rank < partner_rank) {
                        local_data.assign(merged_data.begin(), merged_data.begin() + local_data.size());
                    } else {
                        local_data.assign(merged_data.begin() + local_data.size(), merged_data.end());
                    }
                } else {
                    if (rank < partner_rank) {
                        local_data.assign(merged_data.begin(), merged_data.begin() + local_data.size());
                    } else {
                        local_data.assign(merged_data.begin() + local_data.size(), merged_data.end());
                    }
                }
            }
            MPI_Barrier(comm);
        }
    }

    bitonicSortLocalRecursive(local_data, 0, local_data.size(), true);

    return gatherDataGatherv(local_data, 0, rank, world_size, comm);
}