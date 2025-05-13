# Bitonic Sort Algorithm Documentation

This document provides a detailed explanation of the parallel bitonic sort algorithm implementation in the Parallel Computation Project.

## Overview

The Bitonic Sort algorithm is a comparison-based sorting network that's particularly well-suited for parallel implementations. This implementation uses MPI to distribute the workload across multiple processes, leveraging the algorithm's natural parallelism.

## Header File (`bitonic_sort.h`)

```cpp
#ifndef BITONIC_SORT_H
#define BITONIC_SORT_H

#include <vector>
#include <mpi.h>

using namespace std;

bool isPowerOfTwo(int n);
void compareExchange(vector<int>& arr, int i, int j, bool ascending);
void bitonicMergeLocal(vector<int>& arr, int start, int len, bool ascending);
void bitonicSortLocalRecursive(vector<int>& arr, int start, int len, bool ascending);
vector<int> parallelBitonicSort(vector<int> local_data, int global_data_size,
                              int rank, int world_size, MPI_Comm comm);

#endif
```

- **Header Guards**: Standard inclusion guards to prevent multiple inclusion.
- **Includes**: Vector for data storage and MPI for parallel operations.
- **Function Declarations**:
  - `isPowerOfTwo()`: Checks if a number is a power of 2 (required for bitonic sort).
  - `compareExchange()`: Core operation that compares and swaps elements.
  - `bitonicMergeLocal()`: Merges a bitonic sequence locally.
  - `bitonicSortLocalRecursive()`: Recursive implementation of bitonic sort for local data.
  - `parallelBitonicSort()`: Main function implementing the parallel bitonic sort algorithm.

## Implementation File (`bitonic_sort.cpp`)

### Utility Function: `isPowerOfTwo()`

```cpp
bool isPowerOfTwo(int n) {
    return (n > 0) && ((n & (n - 1)) == 0);
}
```

- **Bit Manipulation**: Uses a bitwise trick to check if a number is a power of 2.
- **Logic**:
  - `n & (n-1)` will be 0 only for powers of 2 (removes the rightmost set bit).
  - Also checks that `n > 0` since 0 would otherwise satisfy the bitwise condition.

### Core Operation: `compareExchange()`

```cpp
void compareExchange(vector<int>& arr, int i, int j, bool ascending) {
    if ((arr[i] > arr[j] && ascending) || (arr[i] < arr[j] && !ascending)) {
        swap(arr[i], arr[j]);
    }
}
```

- **Comparison Logic**: Swaps elements based on the desired sort direction.
- **Parameters**:
  - `arr`: The array containing elements to compare.
  - `i`, `j`: Indices of the elements to compare.
  - `ascending`: Direction flag (true for ascending, false for descending).
- **Swapping**: Conditionally swaps elements if they are out of order.

### Local Merging: `bitonicMergeLocal()`

```cpp
void bitonicMergeLocal(vector<int>& arr, int start, int len, bool ascending) {
    if (len > 1) {
        int k = len / 2;
        for (int i = start; i < start + k; i++) {
            compareExchange(arr, i, i + k, ascending);
        }
        bitonicMergeLocal(arr, start, k, ascending);
        bitonicMergeLocal(arr, start + k, k, ascending);
    }
}
```

- **Recursive Implementation**: Divides the sequence and performs recursive merges.
- **Base Case**: Returns when length is 1 (already sorted).
- **Compare-Exchange Step**: Compares and exchanges elements at distance k.
- **Recursive Calls**: Recursively processes both halves of the sequence.

### Local Sorting: `bitonicSortLocalRecursive()`

```cpp
void bitonicSortLocalRecursive(vector<int>& arr, int start, int len, bool ascending) {
    if (len > 1) {
        int k = len / 2;
        bitonicSortLocalRecursive(arr, start, k, true);       
        bitonicSortLocalRecursive(arr, start + k, k, false);  
        bitonicMergeLocal(arr, start, len, ascending); 
    }
}
```

- **Recursive Bitonic Sort**: Creates a bitonic sequence and then merges it.
- **Sequence Creation**: 
  - Recursively sorts the first half in ascending order.
  - Recursively sorts the second half in descending order.
- **Merge Step**: Merges the two halves using `bitonicMergeLocal()`.
- **Direction Control**: Uses the `ascending` parameter to control sort direction.

### Distributed Merging: `bitonicMergeDistributed()`

```cpp
void bitonicMergeDistributed(vector<int>& local_data, vector<int>& partner_data, bool ascending, bool isLowerRank) {
    vector<int> merged;
    merged.reserve(local_data.size() + partner_data.size());
    
    merge(local_data.begin(), local_data.end(), 
          partner_data.begin(), partner_data.end(), 
          back_inserter(merged));
    
    size_t half_size = local_data.size();
    
    if ((ascending && isLowerRank) || (!ascending && !isLowerRank)) {
        local_data.assign(merged.begin(), merged.begin() + half_size);
    } else {
        local_data.assign(merged.begin() + half_size, merged.end());
    }
}
```

- **Distributed Merge Logic**: Merges data between two processes and keeps appropriate half.
- **Parameters**:
  - `local_data`: The current process's data.
  - `partner_data`: The partner process's data.
  - `ascending`: Sort direction.
  - `isLowerRank`: Flag indicating if this process has lower rank than its partner.
- **Implementation**:
  - Merges both data arrays.
  - Based on rank and sort direction, keeps either the lower or higher half.

### Main Function: `parallelBitonicSort()`

```cpp
vector<int> parallelBitonicSort(vector<int> local_data, int global_data_size_orig,
                              int rank, int world_size, MPI_Comm comm) {
```

- **Parameters**:
  - `local_data`: The portion of data assigned to this process.
  - `global_data_size_orig`: The total size of the original data.
  - `rank`: The current process rank.
  - `world_size`: The total number of processes.
  - `comm`: The MPI communicator.
- **Return Value**: Sorted data (complete array for rank 0, empty for others).

#### Error Checking

```cpp
if (local_data.empty()) {
    if (rank == 0) {
        cout << "Warning: Empty data received for sorting" << endl;
    }
    return vector<int>();
}
```

- **Input Validation**: Handles empty data arrays gracefully.

#### Global Bitonic Sort

```cpp
for (int k = 2; k <= world_size; k *= 2) {
    for (int j = k / 2; j > 0; j /= 2) {
        int partner_rank = rank ^ j;
        
        if (partner_rank >= world_size) {
            continue;
        }
        
        bool ascending_merge = ((rank / k) % 2 == 0);
        bool is_lower_rank = (rank < partner_rank);
```

- **Butterfly Network**: Implements the classic bitonic sort network.
- **Partner Selection**: Uses bitwise XOR to find the partner for each step.
- **Direction Control**: Determines sort direction based on the process's position in the network.
- **Bounds Checking**: Skips iterations where the partner would be out of bounds.

#### Data Exchange

```cpp
MPI_Request send_req, recv_req;
MPI_Status recv_status;
vector<int> partner_data(local_data.size());

int err_recv = MPI_Irecv(partner_data.data(), partner_data.size(), MPI_INT, 
                        partner_rank, 0, comm, &recv_req);

int err_send = MPI_Isend(local_data.data(), local_data.size(), MPI_INT, 
                       partner_rank, 0, comm, &send_req);
```

- **Non-Blocking Communication**: Uses asynchronous send/receive to avoid deadlocks.
- **Buffer Allocation**: Pre-allocates space for partner data.
- **Error Handling**: Captures error codes from MPI operations.

#### Processing Exchanged Data

```cpp
if (err_recv != MPI_SUCCESS || err_send != MPI_SUCCESS) {
    if (rank == 0) {
        cerr << "Error in MPI communication for rank " << rank << endl;
    }
    return vector<int>();
}

MPI_Wait(&recv_req, &recv_status);
MPI_Wait(&send_req, MPI_STATUS_IGNORE);

bitonicMergeDistributed(local_data, partner_data, ascending_merge, is_lower_rank);

MPI_Barrier(comm);
```

- **Communication Completion**: Waits for send/receive operations to complete.
- **Error Handling**: Returns early if communication fails.
- **Data Processing**: Merges local and partner data using the distributed merge function.
- **Synchronization**: Uses a barrier to ensure all processes complete each step.

#### Local Sorting and Result Collection

```cpp
bitonicSortLocalRecursive(local_data, 0, local_data.size(), true);

if (rank == 0) {
    cout << "Local sort complete, gathering results" << endl;
}

vector<int> result = gatherDataGatherv(local_data, 0, rank, world_size, comm);

if (rank == 0 && !result.empty()) {
    sort(result.begin(), result.end());
    cout << "Successfully gathered sorted data (" << result.size() << " elements)" << endl;
}

return result;
```

- **Final Local Sort**: Ensures local data is fully sorted.
- **Result Collection**: Gathers all sorted data to rank 0 using a utility function.
- **Final Check**: Performs a final sort on rank 0 to ensure complete ordering.
- **Return Value**: Returns the sorted array for rank 0 or an empty vector for others.

## Performance Characteristics

- **Time Complexity**: O((n/p) log² n) where n is data size and p is process count.
- **Communication Pattern**: Uses a butterfly network pattern optimized for distributed sorting.
- **Scalability**: Performs well with power-of-two numbers of processes.

## Example Use Case

Bitonic sort is particularly effective for sorting large datasets in parallel computing environments, especially when the number of processes is a power of two. The algorithm guarantees a deterministic sorting pattern regardless of the input data distribution.
