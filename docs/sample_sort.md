# Sample Sort Algorithm Documentation

This document provides a detailed explanation of the parallel sample sort algorithm implementation in the Parallel Computation Project.

## Overview

Sample Sort is a parallel sorting algorithm that uses sampling to partition data effectively across processes. It's especially efficient for large datasets and works well with non-power-of-two numbers of processes.

## Header File (`sample_sort.h`)

```cpp
#ifndef SAMPLE_SORT_H
#define SAMPLE_SORT_H

#include <vector>
#include <mpi.h>

using namespace std;

void merge(vector<int>& arr, int left, int mid, int right);
void mergeSort(vector<int>& arr, int left, int right);
void sortVector(vector<int>& arr);
vector<int> parallelSampleSort(vector<int> local_data, int global_data_size,
                             int rank, int world_size, MPI_Comm comm);

#endif
```

- **Header Guards**: Standard inclusion guards to prevent multiple inclusion.
- **Includes**: Vector for data storage and MPI for parallel operations.
- **Function Declarations**:
  - `merge()`: Helper function to merge two sorted subarrays
  - `mergeSort()`: Recursive divide-and-conquer merge sort implementation
  - `sortVector()`: Wrapper function to sort a complete vector
  - `parallelSampleSort()`: The main function implementing the parallel sample sort algorithm

## Implementation File (`sample_sort.cpp`)

### Helper Functions: Merge Sort Implementation

```cpp
void merge(vector<int>& arr, int left, int mid, int right) {
    int n1 = mid - left + 1;
    int n2 = right - mid;
    vector<int> L(n1), R(n2);

    for (int i = 0; i < n1; i++){
        L[i] = arr[left + i];
    }
    for (int j = 0; j < n2; j++){
        R[j] = arr[mid + 1 + j];
    }
    
    int i = 0, j = 0, k = left;
    while (i < n1 && j < n2) {
        if (L[i] <= R[j]) {
            arr[k] = L[i];
            i++;
        } else {
            arr[k] = R[j];
            j++;
        }
        k++;
    }
    while (i < n1) {
        arr[k] = L[i];
        i++;
        k++;
    }
    while (j < n2) {
        arr[k] = R[j];
        j++;
        k++;
    }
}

void mergeSort(vector<int>& arr, int left, int right) {
    if (left < right) {
        int mid = left + (right - left) / 2;
        mergeSort(arr, left, mid);
        mergeSort(arr, mid + 1, right);
        merge(arr, left, mid, right);
    }
}

void sortVector(vector<int>& arr) {
    if (arr.empty()) return;
    mergeSort(arr, 0, arr.size() - 1);
}
```

- **Merge Function**: Combines two sorted subarrays into one sorted array.
- **Merge Sort Implementation**: Classic divide-and-conquer recursive implementation of merge sort.
- **Helper Wrapper**: `sortVector()` provides a simple interface to sort an entire vector.

### Main Function: `parallelSampleSort()`

```cpp
vector<int> parallelSampleSort(vector<int> local_data, int global_data_size,
                               int rank, int world_size, MPI_Comm comm) {
```

- **Parameters**:
  - `local_data`: The portion of data assigned to this process.
  - `global_data_size`: The total size of the data being sorted.
  - `rank`: The current process rank.
  - `world_size`: The total number of processes.
  - `comm`: The MPI communicator.
- **Return Value**: Sorted data (complete array for rank 0, empty for others).

#### Initial Local Sorting

```cpp
if (global_data_size == 0) return gatherDataGatherv(local_data, 0, rank, world_size, comm);

sortVector(local_data);
```

- **Empty Check**: Handles empty datasets gracefully.
- **Local Sort**: Each process sorts its local data using the merge sort implementation.

#### Sample Selection

```cpp
int num_splitters_per_proc = max(1, world_size -1);
if (world_size == 1) num_splitters_per_proc = 0;

vector<int> local_splitters;
if (!local_data.empty() && num_splitters_per_proc > 0) {
    for (int i = 0; i < num_splitters_per_proc; ++i) {
        local_splitters.push_back(local_data[ (i * local_data.size()) / num_splitters_per_proc ]);
    }
}
```

- **Splitter Count**: Determines how many sample elements to select from each process.
- **Sample Selection**: Selects regularly spaced elements from the local sorted data.
- **Edge Case**: Handles single-process case by setting splitter count to 0.

#### Collecting Splitter Samples

```cpp
int local_splitter_count = local_splitters.size();
vector<int> recv_splitter_counts;
vector<int> splitter_displs;
int total_gathered_splitters = 0;
    
if (rank == 0) {
    recv_splitter_counts.resize(world_size);
    splitter_displs.resize(world_size);
}

MPI_Gather(&local_splitter_count, 1, MPI_INT,
           (rank == 0) ? recv_splitter_counts.data() : nullptr, 1, MPI_INT, 0, comm);
```

- **Preparation**: Each process determines how many splitters it has.
- **Communication**: Uses `MPI_Gather` to collect splitter counts at rank 0.
- **Memory Management**: Only rank 0 allocates space for receiving counts.

#### Gathering All Splitters

```cpp
vector<int> all_splitters_gathered;
if (rank == 0) {
    for (int i = 0; i < world_size; ++i) {
        splitter_displs[i] = (i == 0) ? 0 : splitter_displs[i-1] + recv_splitter_counts[i-1];
        total_gathered_splitters += recv_splitter_counts[i];
    }
    all_splitters_gathered.resize(total_gathered_splitters);
}

MPI_Gatherv(local_splitters.data(), local_splitter_count, MPI_INT,
            (rank == 0) ? all_splitters_gathered.data() : nullptr,
            (rank == 0) ? recv_splitter_counts.data() : nullptr,
            (rank == 0) ? splitter_displs.data() : nullptr,
            MPI_INT, 0, comm);
```

- **Displacement Calculation**: Creates a displacement array for `MPI_Gatherv`.
- **Memory Allocation**: Allocates space for all gathered splitters on rank 0.
- **Communication**: Uses `MPI_Gatherv` to collect variable-length splitter arrays.

#### Selecting Global Splitters

```cpp
vector<int> global_splitters(world_size - 1);
if (rank == 0) {
    // Note: This part was truncated in the provided code
    // It would typically sort all_splitters_gathered and select final splitters
}

if (world_size > 1) {
    MPI_Bcast(global_splitters.data(), world_size - 1, MPI_INT, 0, comm);
}
```

- **Global Splitter Selection**: Rank 0 would select global splitters from the gathered samples.
- **Broadcasting**: Global splitters are broadcast to all processes.
- **Edge Case**: Handles single-process case by skipping the broadcast.

#### Creating Buckets for All-to-All Exchange

```cpp
vector<vector<int>> send_buckets_alltoall(world_size);
if (world_size == 1) {
    send_buckets_alltoall[0] = local_data;
}
else if (!local_data.empty()) {
    // This block would partition local_data into buckets based on splitters
}
```

- **Bucket Creation**: Each process creates buckets based on the global splitters.
- **Data Distribution**: Elements are placed in buckets based on their values relative to the splitters.
- **Edge Case**: For single-process case, all data goes into a single bucket.

#### Preparing for All-to-All Exchange

```cpp
vector<int> send_counts_atoa(world_size);
for (int i = 0; i < world_size; ++i) send_counts_atoa[i] = send_buckets_alltoall[i].size();

vector<int> recv_counts_atoa(world_size);
MPI_Alltoall(send_counts_atoa.data(), 1, MPI_INT, recv_counts_atoa.data(), 1, MPI_INT, comm);

vector<int> send_displs_atoa(world_size, 0);
vector<int> recv_displs_atoa(world_size, 0);
int total_send_size_atoa = 0;
int total_recv_size_atoa = 0;
```

- **Count Exchange**: Each process tells every other process how many elements it will be sending.
- **Displacement Calculation**: Prepares displacement arrays for the all-to-all exchange.

#### Flattening Buckets for Communication

```cpp
vector<int> send_buffer_atoa;
send_buffer_atoa.reserve(local_data.size());
for (int i = 0; i < world_size; ++i) {
    send_displs_atoa[i] = total_send_size_atoa;
    send_buffer_atoa.insert(send_buffer_atoa.end(), send_buckets_alltoall[i].begin(), send_buckets_alltoall[i].end());
    total_send_size_atoa += send_counts_atoa[i];
}
    
for (int i = 0; i < world_size; ++i) {
    recv_displs_atoa[i] = total_recv_size_atoa;
    total_recv_size_atoa += recv_counts_atoa[i];
}
```

- **Buffer Preparation**: Flattens the bucket vectors into a single buffer for communication.
- **Displacement Arrays**: Finalizes displacement arrays for sending and receiving.

#### All-to-All Exchange and Final Sort

```cpp
vector<int> recv_buffer_atoa(total_recv_size_atoa);
MPI_Alltoallv(send_buffer_atoa.data(), send_counts_atoa.data(), send_displs_atoa.data(), MPI_INT,
              recv_buffer_atoa.data(), recv_counts_atoa.data(), recv_displs_atoa.data(), MPI_INT,
              comm);

sortVector(recv_buffer_atoa);
local_data = recv_buffer_atoa;

return gatherDataGatherv(local_data, 0, rank, world_size, comm);
```

- **Data Exchange**: Uses `MPI_Alltoallv` to exchange data so each process gets the elements for its range.
- **Final Local Sort**: Each process sorts the data it received.
- **Result Collection**: Gathers all sorted data to rank 0.

## Performance Characteristics

- **Time Complexity**: Expected O(n/p * log(n/p) + p * log(p)) where n is data size and p is process count.
- **Load Balancing**: Good load balancing when data is well-distributed by the splitters.
- **Scalability**: Works well with non-power-of-two numbers of processes.

## Implementation Notes

1. The sample sort algorithm consists of these main steps:
   - Local sort using merge sort (divide-and-conquer approach)
   - Sample selection
   - Global splitter determination
   - Data redistribution (all-to-all exchange)
   - Final local sort
   - Result gathering

2. The implementation handles various edge cases:
   - Empty data
   - Single-process execution
   - Processes with no data

3. The use of regular sampling helps achieve better load balancing by selecting more representative splitters.

4. The merge sort algorithm was chosen as the local sorting method because:
   - It offers stable O(n log n) performance regardless of input data characteristics
   - It's a classic divide-and-conquer algorithm that divides the array in half recursively
   - It provides predictable performance without the worst-case degradation that can affect quicksort

## Example Use Case

Sample sort is particularly effective for sorting large datasets on distributed systems with many processes. It typically outperforms other parallel sorting algorithms when the number of processes is large and not necessarily a power of two.
