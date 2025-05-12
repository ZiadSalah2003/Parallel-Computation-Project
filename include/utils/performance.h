#ifndef PERFORMANCE_H
#define PERFORMANCE_H

#include <string>
#include <mpi.h>

using namespace std;


void analyzeAndPrintPerformance(const string& algo_name, int data_size,
                                int num_processes, double time_taken, int rank, MPI_Comm comm);

#endif