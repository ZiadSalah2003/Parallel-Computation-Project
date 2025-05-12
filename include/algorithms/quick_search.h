#ifndef QUICK_SEARCH_H
#define QUICK_SEARCH_H

#include <vector>
#include <utility>
#include <mpi.h>

using namespace std;

pair<bool, int> parallelQuickSearch(const vector<int>& local_data, int target_value,
                                   int global_data_size, int rank, int world_size, MPI_Comm comm);

#endif