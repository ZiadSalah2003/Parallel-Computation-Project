#ifndef BITONIC_SORT_H
#define BITONIC_SORT_H

#include <vector>
#include <mpi.h>

using namespace std;

void compareExchange(vector<int>& arr, int i, int j, bool ascending);
void bitonicMergeLocal(vector<int>& arr, int start, int len, bool ascending);
void bitonicSortLocalRecursive(vector<int>& arr, int start, int len, bool ascending);
vector<int> parallelBitonicSort(vector<int> local_data, int global_data_size,
                              int rank, int world_size, MPI_Comm comm);

#endif