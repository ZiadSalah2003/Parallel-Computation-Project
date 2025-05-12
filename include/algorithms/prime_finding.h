#ifndef PRIME_FINDING_H
#define PRIME_FINDING_H

#include <vector>
#include <mpi.h>

using namespace std;
bool isPrime(long long n);
vector<long long> parallelPrimeFinding(long long lower_bound, long long upper_bound,
                                      int rank, int world_size, MPI_Comm comm);

#endif