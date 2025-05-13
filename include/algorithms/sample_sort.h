#ifndef SAMPLE_SORT_H
#define SAMPLE_SORT_H

#include <vector>
#include <mpi.h>

using namespace std;

vector<int> parallelSampleSort(vector<int> local_data, int global_data_size,
                             int rank, int world_size, MPI_Comm comm);

int partition(vector<int>& arr, int low, int high);

void quickSort(vector<int>& arr, int low, int high);

void sortVector(vector<int>& arr);

#endif