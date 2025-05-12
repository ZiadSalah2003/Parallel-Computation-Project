#include "../../include/utils/data_utils.h"
#include <fstream>
#include <vector>
#include <string>
#include <iostream>
#include <mpi.h>

using namespace std;

vector<int> readFileData(const string& filename, int root_rank, int rank, MPI_Comm comm, int& out_global_size) {
    vector<int> data;
    out_global_size = 0;
    if (rank == root_rank) {
        ifstream infile(filename);
        if (!infile) {
            cout << "Error: Could not open input file: " << filename << endl;
        } else {
            int num;
            while (infile >> num) {
                data.push_back(num);
            }
            infile.close();
            out_global_size = data.size();
        }
    }
    MPI_Bcast(&out_global_size, 1, MPI_INT, root_rank, comm);
    return data;
}

void distributeDataScatterv(const vector<int>& global_data, vector<int>& local_data,
                           int root_rank, int rank, int world_size, MPI_Comm comm) {
    int global_size = 0;
    if (rank == root_rank) {
        global_size = global_data.size();
    }
    MPI_Bcast(&global_size, 1, MPI_INT, root_rank, comm);

    if (global_size == 0) {
        local_data.clear();
        return;
    }

    vector<int> sendcounts(world_size);
    vector<int> displs(world_size);
    int chunk_size = global_size / world_size;
    int remainder = global_size % world_size;

    for (int i = 0; i < world_size; ++i) {
        sendcounts[i] = chunk_size + (i < remainder ? 1 : 0);
        displs[i] = (i == 0) ? 0 : displs[i - 1] + sendcounts[i - 1];
    }

    local_data.resize(sendcounts[rank]);
    MPI_Scatterv(global_data.data(), sendcounts.data(), displs.data(), MPI_INT,
                 local_data.data(), sendcounts[rank], MPI_INT,
                 root_rank, comm);
}

vector<int> gatherDataGatherv(const vector<int>& local_data,
                             int root_rank, int rank, int world_size, MPI_Comm comm) {
    int local_size = local_data.size();
    vector<int> recvcounts;
    if (rank == root_rank) {
        recvcounts.resize(world_size);
    }

    MPI_Gather(&local_size, 1, MPI_INT,
               (rank == root_rank) ? recvcounts.data() : nullptr, 1, MPI_INT,
               root_rank, comm);

    vector<int> global_data;
    vector<int> displs;
    int total_size = 0;
    if (rank == root_rank) {
        displs.resize(world_size);
        for (int i = 0; i < world_size; ++i) {
            displs[i] = (i == 0) ? 0 : displs[i - 1] + recvcounts[i - 1];
            total_size += recvcounts[i];
        }
        global_data.resize(total_size);
    }

    MPI_Gatherv(local_data.data(), local_size, MPI_INT,
                (rank == root_rank) ? global_data.data() : nullptr,
                (rank == root_rank) ? recvcounts.data() : nullptr,
                (rank == root_rank) ? displs.data() : nullptr,
                MPI_INT, root_rank, comm);
    return global_data;
}