#include "../../include/algorithms/quick_search.h"
#include <vector>
#include <utility>
#include <mpi.h>

using namespace std;

pair<bool, int> parallelQuickSearch(const vector<int>& local_data, int target_value,
                                    int global_data_size, int rank, int world_size, MPI_Comm comm) {
    bool local_found = false;
    int local_idx = -1;
    int global_idx_offset = 0;

    int chunk_size = global_data_size / world_size;
    int remainder = global_data_size % world_size;
    for(int i = 0; i < rank; ++i) {
        global_idx_offset += chunk_size + (i < remainder ? 1 : 0);
    }

    for (size_t i = 0; i < local_data.size(); ++i) {
        if (local_data[i] == target_value) {
            local_found = true;
            local_idx = global_idx_offset + i;
            break;
        }
    }

    struct {
        int found_flag;
        int index;
    } local_res, global_res;

    local_res.found_flag = local_found ? 1 : 0;
    local_res.index = local_found ? local_idx : global_data_size + 1;

    MPI_Reduce(&local_res, &global_res, 1, MPI_2INT, MPI_MINLOC, 0, comm);

    int found_data[2];
    found_data[0] = local_found ? 1 : 0;
    found_data[1] = local_idx;

    vector<int> all_results_flat;
    if (rank == 0) {
        all_results_flat.resize(2 * world_size);
    }
    MPI_Gather(found_data, 2, MPI_INT, all_results_flat.data(), 2, MPI_INT, 0, comm);

    if (rank == 0) {
        bool final_found = false;
        int final_idx = -1;
        for (int i = 0; i < world_size; ++i) {
            if (all_results_flat[i * 2] == 1) {
                if (!final_found || all_results_flat[i * 2 + 1] < final_idx) {
                    final_found = true;
                    final_idx = all_results_flat[i * 2 + 1];
                }
            }
        }
        return {final_found, final_idx};
    }
    return {false, -1};
}