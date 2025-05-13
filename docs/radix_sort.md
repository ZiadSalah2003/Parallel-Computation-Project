# Radix Sort Algorithm Documentation

This document provides a detailed explanation of the parallel radix sort algorithm implementation in the Parallel Computation Project.

## Overview

The Radix Sort algorithm implements a non-comparative integer sorting method that processes numbers digit by digit. This parallel implementation distributes the workload across multiple processes using MPI.

## Header File (`radix_sort.h`)

```cpp
#ifndef RADIX_SORT_H
#define RADIX_SORT_H

#include <vector>
#include <mpi.h>

using namespace std;

void localCountingSortForRadix(vector<int>& arr, int exp);
vector<int> parallelRadixSort(vector<int> local_data, int global_data_size,
                             int rank, int world_size, MPI_Comm comm);

#endif
```

- **Header Guards**: Prevent multiple inclusion.
- **Includes**: Vector for data storage and MPI for parallel operations.
- **Function Declarations**:
  - `localCountingSortForRadix()`: Helper function that performs counting sort on a specific digit.
  - `parallelRadixSort()`: Main function that implements the parallel radix sort algorithm.

## Implementation File (`radix_sort.cpp`)

### Helper Function: `localCountingSortForRadix()`

```cpp
void localCountingSortForRadix(vector<int>& arr, int exp) {
    if (arr.empty()) 
        return;
    int n = arr.size();
    vector<int> output(n);
    vector<int> count(10, 0);

    for (int i = 0; i < n; i++) 
        count[(arr[i] / exp) % 10]++;

    for (int i = 1; i < 10; i++) 
        count[i] += count[i - 1];
        
    for (int i = n - 1; i >= 0; i--) {
        output[count[(arr[i] / exp) % 10] - 1] = arr[i];
        count[(arr[i] / exp) % 10]--;
    }
    arr = output;
}
```

- **Counting Sort Implementation**: A stable sorting algorithm for a specific digit position.
- **Parameters**:
  - `arr`: The array to be sorted.
  - `exp`: The current digit place value (1, 10, 100, etc.).
- **Early Return**: Handles empty arrays gracefully.
- **Counting Process**:
  1. Count occurrences of each digit (0-9).
  2. Compute cumulative counts to determine positions.
  3. Place elements in output array based on their digit value.
  4. Replace original array with the sorted output.

### Main Function: `parallelRadixSort()`

```cpp
vector<int> parallelRadixSort(vector<int> local_data, int global_data_size,
                            int rank, int world_size, MPI_Comm comm) {
```

- **Parameters**:
  - `local_data`: The portion of data assigned to this process.
  - `global_data_size`: The total size of the data being sorted.
  - `rank`: The current process rank.
  - `world_size`: The total number of processes.
  - `comm`: The MPI communicator.
- **Return Value**: Sorted data (complete array for rank 0, empty for others).

#### Finding Maximum Value

```cpp
int local_max = local_data.empty() ? 0 : *max_element(local_data.begin(), local_data.end());
int global_max = 0;
    
MPI_Allreduce(&local_max, &global_max, 1, MPI_INT, MPI_MAX, comm);
```

- **Local Maximum**: Each process finds the maximum value in its local data.
- **Global Maximum**: `MPI_Allreduce` with `MPI_MAX` operation finds the global maximum across all processes.
- **Empty Data Handling**: Uses 0 as the local maximum for empty arrays.

#### Digit-by-Digit Sorting

```cpp
for (int exp = 1; global_max / exp > 0; exp *= 10) {
    localCountingSortForRadix(local_data, exp);
    MPI_Barrier(comm);
}
```

- **Least-Significant Digit First**: Sorts from the least to most significant digit.
- **Termination Condition**: Stops when there are no more significant digits.
- **Digit Processing**: Calls the counting sort function for each digit place.
- **Synchronization**: Uses `MPI_Barrier` to ensure all processes complete each digit step.

#### Result Collection and Final Sort

```cpp
vector<int> gathered_data = gatherDataGatherv(local_data, 0, rank, world_size, comm);
    
if (rank == 0) {
    for (int exp = 1; global_max / exp > 0; exp *= 10) {
        localCountingSortForRadix(gathered_data, exp);
    }
}
    
return gathered_data;
```

- **Data Gathering**: Collects all locally sorted data to rank 0.
- **Final Sort**: Rank 0 performs a final radix sort on the complete dataset.
- **Result Return**: Returns the sorted array for rank 0 or an empty vector for others.

## Performance Characteristics

- **Time Complexity**: O((n/p) * d) where n is data size, p is process count, and d is the number of digits.
- **Space Complexity**: O(n + k) where k is the range of the input (typically small for digit values).
- **Stability**: The algorithm maintains the relative order of elements with equal keys.

## Implementation Notes

- The current implementation performs a local sort on each process and then gathers the results to the root for a final sort. This is sometimes called a "sort-reduce" approach.
- For very large datasets, an alternative implementation could redistribute data after each digit's sort for better load balancing.
- The algorithm naturally handles positive integers. For negative numbers or floating-point values, additional preprocessing would be required.

## Example Use Case

Radix sort is particularly effective for sorting large datasets of integers with a small range of digit values. This parallel implementation allows it to efficiently handle large datasets by distributing the sorting workload across multiple processes.
