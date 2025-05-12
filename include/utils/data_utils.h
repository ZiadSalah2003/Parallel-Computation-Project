#ifndef DATA_UTILS_H
#define DATA_UTILS_H

#include <vector>
#include <string>
#include <mpi.h>

using namespace std;

vector<int> readFileData(const string& filename, int root_rank, int rank, MPI_Comm comm, int& out_global_size);
void distributeDataScatterv(const vector<int>& global_data, vector<int>& local_data,
                            int root_rank, int rank, int world_size, MPI_Comm comm);
vector<int> gatherDataGatherv(const vector<int>& local_data,
                                  int root_rank, int rank, int world_size, MPI_Comm comm);

#endif