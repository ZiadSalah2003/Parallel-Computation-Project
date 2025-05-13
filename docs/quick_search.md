# Quick Search Algorithm Documentation

This document provides a detailed explanation of the parallel quick search algorithm implementation in the Parallel Computation Project.

## Overview

The Quick Search algorithm implements a parallel search for a target value within distributed data. It's designed to efficiently locate the first occurrence of a value across multiple processes using MPI.

## Header File (`quick_search.h`)

```cpp
#ifndef QUICK_SEARCH_H
#define QUICK_SEARCH_H

#include <vector>
#include <utility>
#include <mpi.h>

using namespace std;

pair<bool, int> parallelQuickSearch(const vector<int>& local_data, int target_value,
                                   int global_data_size, int rank, int world_size, MPI_Comm comm);

#endif
```

- **Header Guards**: `#ifndef`, `#define`, and `#endif` prevent multiple inclusion of the header file.
- **Includes**: 
  - `<vector>`: For storing local data chunks
  - `<utility>`: For the `pair` return type
  - `<mpi.h>`: For parallel processing operations
- **Function Declaration**:
  - `parallelQuickSearch()`: Returns a pair containing a boolean (whether the target was found) and an integer (the index of the first occurrence or -1).

## Implementation File (`quick_search.cpp`)

### Function: `parallelQuickSearch()`

```cpp
pair<bool, int> parallelQuickSearch(const vector<int>& local_data, int target_value,
                                    int global_data_size, int rank, int world_size, MPI_Comm comm) {
```

- **Parameters**:
  - `local_data`: The portion of data assigned to the current process
  - `target_value`: The value to search for
  - `global_data_size`: The total size of the complete data array
  - `rank`: The current process rank
  - `world_size`: The total number of processes
  - `comm`: The MPI communicator
- **Return Value**: A pair containing:
  - A boolean indicating if the target was found
  - The global index of the first occurrence (or -1 if not found)

#### Local Search Preparation

```cpp
bool local_found = false;
int local_idx = -1;
int global_idx_offset = 0;

int chunk_size = global_data_size / world_size;
int remainder = global_data_size % world_size;
for(int i = 0; i < rank; ++i) {
    global_idx_offset += chunk_size + (i < remainder ? 1 : 0);
}
```

- **Initialization**: Sets up initial values for the local search.
- **Index Offset Calculation**: 
  - Determines the starting index of the current process's data in the global array.
  - Uses the same chunk size calculation logic as other algorithms to maintain consistency.
  - Accounts for the remainder by distributing extra elements to lower-ranked processes.

#### Local Search Logic

```cpp
for (size_t i = 0; i < local_data.size(); ++i) {
    if (local_data[i] == target_value) {
        local_found = true;
        local_idx = global_idx_offset + i;
        break;
    }
}
```

- **Search Implementation**: Linear search through the local data chunk.
- **Early Termination**: Stops at the first occurrence of the target value.
- **Global Indexing**: Converts the local index to a global index by adding the offset.

#### Preparing Results for Gathering

```cpp
int found_data[2];
found_data[0] = local_found ? 1 : 0;
found_data[1] = local_found ? local_idx : global_data_size + 1;

vector<int> all_results_flat;
if (rank == 0) {
    all_results_flat.resize(2 * world_size);
}
```

- **Data Packing**: Packs the search result and index into a fixed-size array.
- **Default Value**: Uses `global_data_size + 1` (an invalid index) when the target isn't found.
- **Result Collection**: Only rank 0 allocates space for all results.

#### Gathering Results from All Processes

```cpp
MPI_Gather(found_data, 2, MPI_INT, all_results_flat.data(), 2, MPI_INT, 0, comm);
```

- **Communication**: Uses `MPI_Gather` to collect search results from all processes.
- **Data Structure**: Each process sends two integers (found status and index).

#### Finding the First Global Occurrence

```cpp
if (rank == 0) {
    bool final_found = false;
    int final_idx = -1;
    for (int i = 0; i < world_size; ++i) {
        if (all_results_flat[i * 2] == 1) {
            if (!final_found || all_results_flat[i * 2 + 1] < final_idx) {
                final_found = true;
                final_idx = all_results_flat[i * 2 + 1];
            }
        }
    }
    return {final_found, final_idx};
}
return {false, -1};
```

- **Result Processing**: Rank 0 analyzes all gathered results.
- **First Occurrence**: Finds the minimum valid index among all processes that found the target.
- **Return Value**:
  - Rank 0 returns the final result (whether found and the global index).
  - All other processes return a default negative result.

## Performance Characteristics

- **Time Complexity**: O(n/p) where n is the data size and p is the process count.
- **Communication Cost**: O(p) with a constant factor (2 integers per process).
- **Load Balancing**: Distributes the search workload evenly, handling remainders appropriately.

## Example Use Case

This algorithm efficiently searches for values in large datasets by distributing the search across multiple processes. It's particularly useful for finding the first occurrence of a value in data that's already distributed for other parallel operations.
