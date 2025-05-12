#include <iostream>
#include <fstream>
#include <vector>
#include <random>
#include <cmath>
#include <string>
#include <cstdlib> 


using namespace std;

bool isPowerOfTwo(int n) {
    return (n > 0) && ((n & (n - 1)) == 0);
}

int nextPowerOfTwo(int n) {
    return pow(2, ceil(log2(n)));
}

int main() {
    int num_elements;
    int num_processors;
    string filename;
    
    cout << "Enter the number of elements: ";
    cin >> num_elements;

    
    
    if (!isPowerOfTwo(num_elements)) {
        int next_power = nextPowerOfTwo(num_elements);
        cout << num_elements << " is not a power of two. Using " << next_power << " instead." << endl;
        num_elements = next_power;
    }
    
    cout << "Enter the number of processors: ";
    cin >> num_processors;

    num_elements *= num_processors;

    string default_filename = "bitonic_data_" + to_string(num_elements) + ".txt";
    
    cout << "Enter output filename (default: " << default_filename << "): ";
    cin.ignore(); 
    getline(cin, filename);
    
    if (filename.empty()) {
        filename = default_filename;
    }
    
    vector<int> data(num_elements);
   
    
    for (int i = 0; i < num_elements; i++) {
        data[i] = rand() % num_elements + 1; 
    }
    
    ofstream outfile(filename);
    if (!outfile.is_open()) {
        cerr << "Error opening file: " << filename << endl;
        return 1;
    }
    
    for (int i = 0; i < num_elements; i++) {
        if (i) {
            outfile << " ";
        }
        outfile << data[i];
    }
    outfile << endl;
    
    cout << "Successfully generated " << num_elements << " elements for bitonic sort in " << filename << endl;
    
    return 0;
}