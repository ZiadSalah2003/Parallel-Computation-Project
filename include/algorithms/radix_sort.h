#ifndef RADIX_SORT_H
#define RADIX_SORT_H

#include <vector>
#include <mpi.h>

using namespace std;

int getRadixMax(const vector<int>& arr);
void localCountingSortForRadix(vector<int>& arr, int exp);
vector<int> parallelRadixSort(vector<int> local_data, int global_data_size,
                             int rank, int world_size, MPI_Comm comm);

#endif