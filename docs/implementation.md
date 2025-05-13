# Implementation Details

This document provides detailed information about the implementation of each parallel algorithm in the project.

## Quick Search

The Quick Search algorithm implements a parallel approach to finding a target value in an array:

1. **Data Distribution**:

   - The input array is divided into approximately equal chunks
   - Each process receives its portion using `MPI_Scatterv`

2. **Local Search**:

   - Each process searches for the target value in its local portion
   - Records the local index if found

3. **Result Aggregation**:

   - Using `MPI_Reduce` with `MPI_MINLOC` to find the first occurrence
   - Additional `MPI_Gather` to collect all results for verification

4. **Time Complexity**: O(n/p) where n is the data size and p is number of processes

5. **Implementation Notes**:
   - Handles edge cases gracefully when target is not found
   - Maintains global indexing for consistent results

## Prime Number Finding

The Prime Finding algorithm uses a parallel approach to efficiently find all prime numbers within a given range:

1. **Range Partitioning**:

   - Divides the range [lower_bound, upper_bound] into roughly equal segments
   - Handles remainder by distributing extra elements to lower-ranked processes

2. **Primality Testing**:

   - Each process tests numbers in its assigned range
   - Local primes are collected in a vector

3. **Result Collection**:

   - Uses `MPI_Gather` to get counts of primes found by each process
   - Then uses `MPI_Gatherv` to collect the actual prime numbers
   - Root process sorts the final list of primes

4. **Time Complexity**: O((m/p) \* √n) where m is range size, p is process count, and n is the largest number

5. **Implementation Notes**:
   - Uses an efficient primality test with √n optimization
   - Output is written to a file

## Bitonic Sort

The Bitonic Sort implementation leverages the parallel nature of this algorithm:

1. **Initial Setup**:

   - Requires power-of-two size data (adjusted during data generation)
   - Data is distributed evenly using `MPI_Scatterv`
   - Each process locally sorts its portion

2. **Bitonic Sequence Creation and Merging**:

   - Implements the butterfly network communication pattern
   - Each process finds its partner at each step using bitwise XOR
   - Exchanges data with partner using `MPI_Sendrecv`
   - Performs appropriate comparisons based on the sorting direction

3. **Local Recursive Bitonic Sort**:

   - Final stage uses recursive bitonic sorting on local data
   - Ensures each process's data is properly sorted

4. **Time Complexity**: O((n/p) log² n) where n is data size and p is process count

5. **Implementation Notes**:
   - Special handling for partner identification to avoid out-of-bounds ranks
   - Optimized merge operations for improved performance

## Radix Sort

The Radix Sort implementation uses a digit-by-digit approach in parallel:

1. **Data Distribution**:

   - Input data is distributed among processes using `MPI_Scatterv`

2. **Local Counting Sort**:

   - Each process performs counting sort on its local data for each digit position
   - Processes from least significant to most significant digit

3. **Global Coordination**:

   - Determines maximum value to know how many digit positions to process
   - Uses `MPI_Allreduce` to find global maximum across all processes

4. **Time Complexity**: O((n/p) \* d) where n is data size, p is process count, and d is the number of digits

5. **Implementation Notes**:
   - Current implementation handles the first few digit positions and sorts the rest with a standard sort
   - Scope for optimization in the full digit-by-digit redistributed implementation

## Sample Sort

The Sample Sort implementation provides efficient sorting even with non-power-of-two data:

1. **Local Sorting**:

   - Each process sorts its local data
   - Selects regular samples from its sorted local data

2. **Splitter Selection**:

   - Gathers samples from all processes at the root
   - Root process sorts all samples and selects global splitters
   - Broadcasts splitters to all processes

3. **Data Redistribution**:

   - Each process partitions its data according to splitters
   - Uses `MPI_Alltoall` to exchange counts of data being sent
   - Uses `MPI_Alltoallv` for actual data redistribution

4. **Final Local Sorting**:

   - Each process sorts its received data
   - Results are gathered at the root using `MPI_Gatherv`

5. **Time Complexity**: O((n/p) log(n/p) + p² log p) where n is data size and p is process count

6. **Implementation Notes**:
   - Handles edge cases like empty partitions
   - Optimized to minimize communication overhead
