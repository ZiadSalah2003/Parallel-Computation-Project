#ifndef SAMPLE_SORT_H
#define SAMPLE_SORT_H

#include <vector>
#include <mpi.h>

using namespace std;

void merge(vector<int>& arr, int left, int mid, int right);
void mergeSort(vector<int>& arr, int left, int right);
void sortVector(vector<int>& arr);
vector<int> parallelSampleSort(vector<int> local_data, int global_data_size,
                             int rank, int world_size, MPI_Comm comm);

#endif