#include <iostream>
#include <fstream>
#include "types.hpp"
#include "parser.hpp"
#include "grid.hpp"
#include "mfmc.hpp"

using namespace std;

int main(int argc, char* argv[]){

    // check arguments
    if (argc != 3) {
        cerr << "usage: cts <in_file> <out_file>\n";
        return 1;
    }
    const char* inFile = argv[1];
    const char* outFile = argv[2];
    ofstream out(outFile);
    if (!out) {
        cerr << "Error: could not open output file " << outFile << "\n";
    }

    // parse problem
    Problem prob = ParseProblem(inFile);

    // initialize grid
    Grid grid;
    grid.gridInit(prob);

    // assign pins to taps
    MFMC mfmc;
    map<int, int> assignment = mfmc.assignPinsToTaps(prob);
    //output: matching result

    return 0;
}
