#include "../../include/utils/performance.h"
#include <iostream>
#include <iomanip>
#include <string>
#include <mpi.h>

using namespace std;



void analyzeAndPrintPerformance(const string& algo_name, int data_size,
                                int num_processes, double time_taken, int rank, MPI_Comm comm) {
    if (rank == 0) { 
        cout << "\n--- Performance Analysis for " << algo_name << " ---" << endl;
        cout << "Data Size / Range: " << data_size << " elements/items" << endl;
        cout << "Number of Processes: " << num_processes << endl;
        cout << fixed << setprecision(6);
        cout << "Time Taken: " << time_taken << " seconds (" << time_taken * 1000.0 << " ms)" << endl;
        cout << "--------------------------------------------------" << endl;
    }
}