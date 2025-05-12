#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <algorithm>
#include <cmath>
#include <mpi.h>
#include <iomanip>
#include <numeric>
#include <sys/stat.h>

#include "../include/algorithms/quick_search.h"
#include "../include/algorithms/prime_finding.h"
#include "../include/algorithms/bitonic_sort.h"
#include "../include/algorithms/radix_sort.h"
#include "../include/algorithms/sample_sort.h"
#include "../include/utils/data_utils.h"
#include "../include/utils/performance.h"

using namespace std;


int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);
    int rank, world_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    const int root_rank = 0;
    char try_again = 'Y';

    do {
        int choice = 0;
        string input_filename;
        vector<int> global_data_vec;
        vector<int> local_data_vec;
        int global_vec_size = 0;
        int search_target = 0;
        long long prime_lower = 0, prime_upper = 0;

        if (rank == root_rank) {
            cout << "\n================================================" << endl;
            cout << "Welcome to Parallel Algorithm Simulation with MPI" << endl;
            cout << "================================================" << endl;
            cout << "Running on " << world_size << " processes." << endl;
            cout << "\nPlease choose an algorithm to execute:" << endl;
            cout << "1 - Quick Search" << endl;
            cout << "2 - Prime Number Finding" << endl;
            cout << "3 - Bitonic Sort" << endl;
            cout << "4 - Radix Sort" << endl;
            cout << "5 - Sample Sort" << endl;
            cout << "Enter the number of the algorithm to run: ";
            cin >> choice;

            if (choice == 1 || choice == 3 || choice == 4 || choice == 5) {
                cout << "Please enter the path to the input file: ";
                cin >> input_filename;
            }
            if (choice == 1) {
                cout << "Enter Search Target: ";
                cin >> search_target;
            } else if (choice == 2) {
                cout << "Enter the lower bound for prime number finding: ";
                cin >> prime_lower;
                cout << "Enter the upper bound for prime number finding: ";
                cin >> prime_upper;
                 if (prime_upper < prime_lower) {
                    cout << "Error: Upper bound cannot be less than lower bound." << endl;
                    choice = -1;
                }
            }
            else if (choice == 3 && !isPowerOfTwo(world_size)) {
                cout << "Error: Number of processes is not a power of 2." << endl;
                choice = -1;
            }
        }

        MPI_Bcast(&choice, 1, MPI_INT, root_rank, MPI_COMM_WORLD);
        
        if (choice == -1) { 
            MPI_Finalize();
            return 1;
        }

        if (choice == 1 || choice == 3 || choice == 4 || choice == 5) {
            
            int filename_len = 0;
            
            if (rank == root_rank) {
                filename_len = input_filename.length();
            }

            MPI_Bcast(&filename_len, 1, MPI_INT, root_rank, MPI_COMM_WORLD);
            
            if (rank != root_rank) {
                input_filename.resize(filename_len);
            }


            MPI_Bcast(&input_filename[0], filename_len, MPI_CHAR, root_rank, MPI_COMM_WORLD);

            if (rank == root_rank) {
                cout << "Reading data from file..." << endl;
            }

            global_data_vec = readFileData(input_filename, root_rank, rank, MPI_COMM_WORLD, global_vec_size);

            if (global_vec_size == 0 && choice != 2) { 
                 if (rank == root_rank) {
                    cout << "Error: Input file is empty or could not be read for array-based algorithms." << endl;
                 }

                 MPI_Bcast(&try_again, 1, MPI_CHAR, root_rank, MPI_COMM_WORLD); 
            }
        }
        
        if (choice == 1) {
            MPI_Bcast(&search_target, 1, MPI_INT, root_rank, MPI_COMM_WORLD);
        }

        if (choice == 2) {
            MPI_Bcast(&prime_lower, 1, MPI_LONG_LONG, root_rank, MPI_COMM_WORLD);
            MPI_Bcast(&prime_upper, 1, MPI_LONG_LONG, root_rank, MPI_COMM_WORLD);
        }

        if ((choice == 1 || choice == 3 || choice == 4 || choice == 5) && global_vec_size > 0) {
            if (rank == root_rank) cout << "Distributing data across processes..." << endl;
            distributeDataScatterv(global_data_vec, local_data_vec, root_rank, rank, world_size, MPI_COMM_WORLD);
        }

        MPI_Barrier(MPI_COMM_WORLD);
        double start_time = MPI_Wtime();
        double end_time;
        vector<int> result_vec;
        vector<long long> result_primes_vec;


        switch (choice) {
            case 1: { 
                if (rank == root_rank) {
                    cout << "\nQuick Search Selected" << endl;
                }

                 if (global_vec_size == 0) {
                    if(rank == root_rank) 
                        cout << "Result: No data to search in." << endl;
                    break;
                }

                if (rank == root_rank) {
                    cout << "Each process is searching its assigned range..." << endl;
                }

                pair<bool, int> search_res = parallelQuickSearch(local_data_vec, search_target, global_vec_size, rank, world_size, MPI_COMM_WORLD);
                end_time = MPI_Wtime();


                if (rank == root_rank) {
                    if (search_res.first) {
                        cout << "Result: Value " << search_target << " found at global index " << search_res.second << endl;
                    } else {
                        cout << "Result: Value " << search_target << " not found." << endl;
                    }
                }
                break;
            }
            case 2: { 
                if (rank == root_rank) {
                    cout << "\nPrime Number Finding Selected" << endl;
                }

                result_primes_vec = parallelPrimeFinding(prime_lower, prime_upper, rank, world_size, MPI_COMM_WORLD);
                end_time = MPI_Wtime();
                if (rank == root_rank) {
                    cout << "Result: Found " << result_primes_vec.size() << " prime numbers between "
                              << prime_lower << " and " << prime_upper << "." << endl;
                     ofstream outfile("output/primes.txt");
                   
                     if (outfile.is_open()) {
                        for (long long p : result_primes_vec) outfile << p << "\n";
                        outfile.close();
                        cout << "All prime numbers stored in output/primes.txt" << endl;
                    }
                }
                break;
            }
            case 3:   
            case 4:   
            case 5: { 
                 if (global_vec_size == 0) {
                    if(rank == root_rank) cout << "Result: No data to sort." << endl;
                    break;
                }

                string algo_name;
                string out_filename;
                if (choice == 3) { 
                    algo_name = "Bitonic Sort"; out_filename = "output/bitonic_sort.txt"; 
                }

                if (choice == 4) { 
                    algo_name = "Radix Sort"; out_filename = "output/radix_sort.txt";
                }

                if (choice == 5) { 
                    algo_name = "Sample Sort (Bonus)"; out_filename = "output/sample_sort.txt";
                }

                if (rank == root_rank) cout << "\n" << algo_name << " Selected" << endl;
                if (choice == 4 && rank == root_rank) {
                     cout << "Each process is sorting based on digit position..." << endl;
                     cout << "Performing digit-wise counting and merging..." << endl;
                }

                if (choice == 3) {
                    result_vec = parallelBitonicSort(local_data_vec, global_vec_size, rank, world_size, MPI_COMM_WORLD);
                }

                else if (choice == 4) {
                    result_vec = parallelRadixSort(local_data_vec, global_vec_size, rank, world_size, MPI_COMM_WORLD);
                }

                else if (choice == 5){
                    result_vec = parallelSampleSort(local_data_vec, global_vec_size, rank, world_size, MPI_COMM_WORLD);
                }

                end_time = MPI_Wtime();

                if (rank == root_rank) {
                    if (!result_vec.empty()) {
                        ofstream outfile(out_filename);
                        if (outfile.is_open()) {
                            for (size_t i = 0; i < result_vec.size(); ++i) {
                                outfile << result_vec[i] << (i == result_vec.size() - 1 ? "" : " ");
                            }
                            outfile.close();
                            cout << "Sorted array stored in " << out_filename << endl;
                        }
                    } else {
                        cout << "Result: Sorting produced an empty list or failed." << endl;
                    }
                }
                break;
            }
            default:
                if (rank == root_rank) cout << "Invalid choice." << endl;
                end_time = start_time; 
                break;
        }

        MPI_Barrier(MPI_COMM_WORLD); 
        double elapsed_time = end_time - start_time;


        if (choice >= 1 && choice <= 5) {
            string name = "";
            int data_items = 0;
            if(choice == 1) { 
                name = "Quick Search"; data_items = global_vec_size; 
            }
            if(choice == 2) { 
                name = "Prime Number Finding"; data_items = prime_upper - prime_lower + 1;
            }
            if(choice == 3) {
                name = "Bitonic Sort"; data_items = global_vec_size; 
            }
            if(choice == 4) {
                name = "Radix Sort"; data_items = global_vec_size;
            }
            if(choice == 5) {
                name = "Sample Sort"; data_items = global_vec_size;
            }

            if (!( (choice == 1 || choice >=3) && global_vec_size == 0) ) {
                 analyzeAndPrintPerformance(name, data_items, world_size, elapsed_time, rank, MPI_COMM_WORLD);
            }
        }


        if (rank == root_rank) {
            cout << "\nWant to try another algorithm? (Y/N): ";
            cin >> try_again;
        }
        MPI_Bcast(&try_again, 1, MPI_CHAR, root_rank, MPI_COMM_WORLD);

    } while (try_again == 'Y' || try_again == 'y');

    if (rank == root_rank) {
        cout << "Exiting program." << endl;
    }

    MPI_Finalize();
    return 0;
}