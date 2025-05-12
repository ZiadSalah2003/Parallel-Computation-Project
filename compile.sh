#!/bin/bash

echo "Compiling Parallel Computation Project"

mkdir -p output

echo "Compiling with mpic++..."
mpic++ -I./include src/main.cpp src/algorithms/*.cpp src/utils/*.cpp -o parallel_computation.o