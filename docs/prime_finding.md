# Prime Number Finding Algorithm Documentation

This document provides a detailed explanation of the parallel prime number finding algorithm implementation in the Parallel Computation Project.

## Overview

The prime finding algorithm finds all prime numbers within a specified range using MPI for parallel processing. The implementation distributes the workload across available processes, with each process checking a subset of the range for prime numbers.

## Header File (`prime_finding.h`)

```cpp
#ifndef PRIME_FINDING_H
#define PRIME_FINDING_H

#include <vector>
#include <mpi.h>

using namespace std;
bool isPrime(long long n);
vector<long long> parallelPrimeFinding(long long lower_bound, long long upper_bound,
                                      int rank, int world_size, MPI_Comm comm);

#endif
```

- **Header Guards**: `#ifndef`, `#define`, and `#endif` prevent multiple inclusion of the header file.
- **Includes**: Standard library vector and MPI header for parallel processing.
- **Function Declarations**:
  - `isPrime(long long n)`: A helper function that checks if a number is prime.
  - `parallelPrimeFinding()`: The main function that implements parallel prime number finding.

## Implementation File (`prime_finding.cpp`)

### Function: `isPrime()`

```cpp
bool isPrime(long long n) {
    if (n <= 1) 
        return false;
    if (n == 2) 
        return true;

    for (long long i = 2; i * i <= n; i++) {
        if (n % i == 0) 
            return false;
    }
    return true;
}
```

- **Base Cases**:
  - Numbers less than or equal to 1 are not prime, so return `false`.
  - 2 is the only even prime number, so return `true`.
- **Optimization**: Checks divisors only up to the square root of `n` (expressed as `i * i <= n`).
- **Logic**: If any number divides `n` without a remainder, `n` is not prime.

### Function: `parallelPrimeFinding()`

```cpp
vector<long long> parallelPrimeFinding(long long lower_bound, long long upper_bound,
                                     int rank, int world_size, MPI_Comm comm) {
```
- **Parameters**:
  - `lower_bound`: The lower limit of the range to check for prime numbers.
  - `upper_bound`: The upper limit of the range to check.
  - `rank`: The rank of the current process.
  - `world_size`: The total number of available MPI processes.
  - `comm`: The MPI communicator.
- **Return Value**: A vector containing all prime numbers in the range (for rank 0), or an empty vector (for other ranks).

#### Range Partitioning

```cpp
vector<long long> local_primes;
long long total_numbers = upper_bound - lower_bound + 1;
    
if (total_numbers <= 0) 
    return {};

long long chunk_size = total_numbers / world_size;
long long remainder = total_numbers % world_size;

long long my_start = lower_bound + rank * chunk_size + min((long long)rank, remainder);
long long my_end = my_start + chunk_size + (rank < remainder ? 1 : 0) -1;
```

- **Input Validation**: Returns an empty vector if the range is invalid.
- **Load Balancing**: 
  - Calculates the basic chunk size by dividing the range by the number of processes.
  - Handles any remainder by distributing an extra number to lower-ranked processes.
- **Range Calculation**: 
  - Each process calculates its own start and end points based on its rank.
  - The formula ensures that extra elements (from the remainder) are distributed to lower-ranked processes.

#### Primality Testing

```cpp
for (long long i = my_start; i <= my_end; ++i) {
    if (isPrime(i)) {
        local_primes.push_back(i);
    }
}
```

- **Local Processing**: Each process tests each number in its assigned range.
- **Result Collection**: If a number is prime, it's added to the local result vector.

#### Gathering Results

```cpp
int local_size = local_primes.size();
vector<int> recv_counts;
if (rank == 0) 
    recv_counts.resize(world_size);

MPI_Gather(&local_size, 1, MPI_INT, (rank == 0) ? recv_counts.data() : nullptr, 1, MPI_INT, 0, comm);
```

- **Preparation**: Each process determines how many primes it found.
- **Communication**: Using `MPI_Gather` to collect the counts of primes from all processes.
- **Memory Management**: Only rank 0 allocates memory for receiving counts.

#### Array Displacement Calculation

```cpp
vector<long long> global_primes;
vector<int> displs;
if (rank == 0) {
    displs.resize(world_size);
    int total_gathered_primes = 0;
    for (int i = 0; i < world_size; ++i) {
        displs[i] = (i == 0) ? 0 : displs[i - 1] + recv_counts[i - 1];
        total_gathered_primes += recv_counts[i];
    }
    global_primes.resize(total_gathered_primes);
}
```

- **Offset Calculation**: Creates a displacement array for `MPI_Gatherv` to properly place each process's data.
- **Result Space**: Allocates memory for the global result based on the total count of primes.

#### Collecting Prime Numbers

```cpp
MPI_Gatherv(local_primes.data(), local_size, MPI_LONG_LONG,
            (rank == 0) ? global_primes.data() : nullptr,
            (rank == 0) ? recv_counts.data() : nullptr,
            (rank == 0) ? displs.data() : nullptr,
            MPI_LONG_LONG, 0, comm);

if (rank == 0) {
    return global_primes;
}
return {};
```

- **Variable-Length Data Exchange**: Uses `MPI_Gatherv` to collect prime numbers from all processes.
- **Result Return**: Only rank 0 returns the complete list of prime numbers; other processes return an empty vector.

## Performance Characteristics

- **Time Complexity**: O((m/p) * √n), where m is the range size, p is the process count, and n is the largest number.
- **Load Balancing**: Handles uneven distribution by allocating extra work to lower-ranked processes.
- **Optimization**: Uses an efficient primality test checking only up to the square root of each number.

## Example Use Case

This algorithm can efficiently find all prime numbers in large ranges by distributing the workload across multiple processes. For instance, finding all primes between 1 and 1,000,000 can be significantly faster with multiple processes compared to a sequential approach.
