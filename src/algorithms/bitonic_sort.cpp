#include "../../include/algorithms/bitonic_sort.h"
#include "../../include/utils/data_utils.h"
#include <algorithm>
#include <vector>
#include <mpi.h>
#include <iostream>
#include <cmath>

using namespace std;

bool isPowerOfTwo(int n) {
    return (n > 0) && ((n & (n - 1)) == 0);
}

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

void bitonicMergeDistributed(vector<int>& local_data, vector<int>& partner_data, bool ascending, bool isLowerRank) {

    vector<int> merged;
    merged.reserve(local_data.size() + partner_data.size());
    
    merge(local_data.begin(), local_data.end(), 
          partner_data.begin(), partner_data.end(), 
          back_inserter(merged));
    
    size_t half_size = local_data.size();
    
    if ((ascending && isLowerRank) || (!ascending && !isLowerRank)) {
        local_data.assign(merged.begin(), merged.begin() + half_size);
    } else {
        local_data.assign(merged.begin() + half_size, merged.end());
    }
}

vector<int> parallelBitonicSort(vector<int> local_data, int global_data_size_orig,
                              int rank, int world_size, MPI_Comm comm) {

    
    if (local_data.empty()) {
        if (rank == 0) {
            cout << "Warning: Empty data received for sorting" << endl;
        }
        return vector<int>();
    }
    
    
    for (int k = 2; k <= world_size; k *= 2) {
        for (int j = k / 2; j > 0; j /= 2) {
            int partner_rank = rank ^ j;
            
            if (partner_rank >= world_size) {
                continue;
            }
            
            
            bool ascending_merge = ((rank / k) % 2 == 0);
            bool is_lower_rank = (rank < partner_rank);
            
            MPI_Request send_req, recv_req;
            MPI_Status recv_status;
            vector<int> partner_data(local_data.size());
            
            int err_recv = MPI_Irecv(partner_data.data(), partner_data.size(), MPI_INT, 
                                    partner_rank, 0, comm, &recv_req);
            
            int err_send = MPI_Isend(local_data.data(), local_data.size(), MPI_INT, 
                                   partner_rank, 0, comm, &send_req);
            
            if (err_recv != MPI_SUCCESS || err_send != MPI_SUCCESS) {
                if (rank == 0) {
                    cerr << "Error in MPI communication for rank " << rank << endl;
                }
                return vector<int>();
            }
            
            MPI_Wait(&recv_req, &recv_status);
            
            MPI_Wait(&send_req, MPI_STATUS_IGNORE);
            
            bitonicMergeDistributed(local_data, partner_data, ascending_merge, is_lower_rank);
            
            MPI_Barrier(comm);
        }
    }

    bitonicSortLocalRecursive(local_data, 0, local_data.size(), true);

    if (rank == 0) {
        cout << "Local sort complete, gathering results" << endl;
    }

    vector<int> result = gatherDataGatherv(local_data, 0, rank, world_size, comm);
    
    if (rank == 0 && !result.empty()) {
        sort(result.begin(), result.end());
        cout << "Successfully gathered sorted data (" << result.size() << " elements)" << endl;
    }
    
    return result;
}