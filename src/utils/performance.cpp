#include "../../include/utils/performance.h"
#include <iostream>
#include <iomanip>
#include <string>
#include <mpi.h>

using namespace std;

double sequential_time = 0.0;

void recordSequentialTimeIfSingleProcess(double parallel_time, int world_size) {
    if (world_size == 1) {
        sequential_time = parallel_time;
    }
  
    MPI_Bcast(&sequential_time, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
}

void analyzeAndPrintPerformance(const string& algo_name, int data_size,
                                int num_processes, double time_taken, int rank, MPI_Comm comm) {
    if (rank == 0) { 
        cout << "\n--- Performance Analysis for " << algo_name << " ---" << endl;
        cout << "Data Size / Range: " << data_size << " elements/items" << endl;
        cout << "Number of Processes: " << num_processes << endl;
        cout << fixed << setprecision(6);
        cout << "Time Taken: " << time_taken << " seconds (" << time_taken * 1000.0 << " ms)" << endl;

        if (sequential_time > 0.000001 && num_processes > 0) { 
            double speedup = sequential_time / time_taken;
            double efficiency = speedup / num_processes;
            cout << "Sequential Time (for P=1): " << sequential_time << " seconds" << endl;
            cout << "Speedup: " << speedup << endl;
            cout << "Efficiency: " << efficiency * 100 << "%" << endl;
        } else if (num_processes == 1) {
             cout << "(This run was with P=1, considered as sequential baseline for future runs if `sequential_time` was updated)" << endl;
        }
         else {
            cout << "(Sequential time not available or P=0, cannot calculate speedup/efficiency)" << endl;
        }
        cout << "--------------------------------------------------" << endl;
    }
}