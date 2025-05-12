#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdlib> 
#include <ctime>

using namespace std;

int main() {
    int num_elements;
    string filename;
    
    cout << "Enter the number of elements: ";
    cin >> num_elements;
    cin.ignore();
        
    
    string default_filename = "data_"  + to_string(num_elements) + ".txt";
    
    cout << "Enter output filename (default: " << default_filename << "): ";
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
    
    cout << "Successfully generated " << num_elements << " in " << filename << endl;
    
    return 0;
}