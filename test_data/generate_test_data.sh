#!/bin/bash


echo "Compiling data generators..."
g++ -o generate_power_of_two_data.o generate_power_of_two_data.cpp
g++ -o generate_general_data.o generate_general_data.cpp

echo ""
echo "===== TEST DATA GENERATOR ====="
echo "1. Generate power-of-two data for Bitonic Sort"
echo "2. Generate data for other algorithms"
echo "============================="
echo "Enter your choice (1-2): "
read choice

case $choice in
    1)
        echo "Running power-of-two data generator for Bitonic Sort..."
        ./generate_power_of_two_data.o
        ;;
    2)
        echo "Running general data generator for other algorithms..."
        ./generate_general_data.o
        ;;
    *)
        echo "Invalid choice"
        ;;
esac

echo "Done."