# Parallel Computation Project with MPI

This project implements various parallel search and sorting algorithms using the Message Passing Interface (MPI) standard.

## Overview

This project includes the following parallel algorithms:

1. **Quick Search** - A parallel implementation of search algorithm
2. **Prime Number Finding** - Parallel prime number detection within a given range
3. **Bitonic Sort** - A parallel sorting algorithm optimized for power-of-two sized arrays
4. **Radix Sort** - A parallel implementation of the radix sorting algorithm
5. **Sample Sort** - A parallel sorting algorithm that works efficiently with non-power-of-two processes

## Prerequisites

- C++ Compiler with C++11 support or higher
- MPI Implementation (MPICH or OpenMPI)


## Building the Project

1. Clone this repository
2. Navigate to the project directory
3. Run the compile script:

```bash
chmod +x compile.sh
./compile.sh
```

This will generate the executable `parallel_computation.o`.

## Running the Project

### On a Single Machine (Multiple Processes)

```bash
mpirun -np <number_of_processes> ./parallel_computation.o
```

Replace `<number_of_processes>` with the desired number of MPI processes.

### On a Cluster (Multiple Nodes)

```bash
mpirun -hostfile hostfile ./parallel_computation.o
```

The `hostfile` contains the list of nodes and the number of slots (processes) per node.

## Generating Test Data

The project includes tools to generate test data for the algorithms:

```bash
cd test_data
chmod +x generate_test_data.sh
./generate_test_data.sh
```

This script offers two options:

1. Generate power-of-two sized data for Bitonic Sort
2. Generate general data for other algorithms

## Project Structure

```
.
├── compile.sh               # Compilation script
├── doc/                     # Documentation directory
├── hostfile                 # MPI hostfile for cluster configuration
├── include/                 # Header files
│   ├── algorithms/          # Algorithm headers
│   └── utils/               # Utility headers
├── output/                  # Output directory for results
├── README.md                # This file
├── src/                     # Source files
│   ├── algorithms/          # Algorithm implementations
│   ├── main.cpp             # Main application entry point
│   └── utils/               # Utility implementations
└── test_data/               # Test data generation tools
```

## Algorithm Descriptions

### Quick Search

A parallel search algorithm that distributes the search space across multiple processes to find a target value efficiently.

### Prime Number Finding

Divides the range among available processes to find prime numbers in parallel, significantly improving performance for large ranges.

### Bitonic Sort

A comparison-based sorting algorithm that produces a bitonic sequence and then efficiently transforms it into a sorted sequence. Works best with power-of-two sized arrays and processes.

### Radix Sort

A non-comparative integer sorting algorithm that processes integer keys based on individual digits with the same significant position and value.

### Sample Sort

A sorting algorithm that determines splitters based on samples from all processes, then redistributes elements to appropriate processes for final sorting.

## Performance Analysis

Performance metrics are automatically collected and displayed after each algorithm execution. To understand detailed performance analysis, refer to the documentation in the `doc/` directory.