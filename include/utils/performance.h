#ifndef PERFORMANCE_H
#define PERFORMANCE_H

#include <string>
#include <mpi.h>

using namespace std;

extern double sequential_time;

void recordSequentialTimeIfSingleProcess(double parallel_time, int world_size);
void analyzeAndPrintPerformance(const string& algo_name, int data_size,
                                int num_processes, double time_taken, int rank, MPI_Comm comm);

#endif