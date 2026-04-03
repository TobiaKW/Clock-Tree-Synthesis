#include <iostream>
#include <fstream>
#include <sstream>
#include "parser.hpp"

using namespace std;

Problem ParseProblem(const char* filename) {
    ifstream file(filename);
    if (!file) {
        cerr << "Error: could not open file " << filename << "\n";
        return {};
    }

    Problem prob;
    string line, word;

    while (getline(file, line)) {
        if (line.empty()) {
            continue;
        }
        istringstream iss(line);
        iss >> word;
        if (word == "MAX_LOAD") {
            iss >> prob.MAX_LOAD; 
        } else if (word == "GRID_SIZE") {
            iss >> prob.GRID_SIZE;
        } else if (word == "CAPACITY") {
            iss >> prob.CAPACITY;
        } else if (word == "TAPS") {
            iss >> prob.numTaps;
            prob.taps.resize(prob.numTaps);
            
            for (int i = 0; i < prob.numTaps; i++) {
                getline(file, line);
                istringstream tapStream(line);
                string tapWord;
                tapStream >> tapWord;
                
                if (tapWord == "TAP") {
                    tapStream >> prob.taps[i].id >> prob.taps[i].x >> prob.taps[i].y;
                }
                else{
                    cerr << "Error: invalid input in TAPS" << endl;
                    return {};
                }
            }
        } else if (word == "PINS") {
            iss >> prob.numPins;
            prob.pins.resize(prob.numPins);
            
            for (int i = 0; i < prob.numPins; i++) {
                getline(file, line);
                istringstream pinStream(line);
                string pinWord;
                pinStream >> pinWord;
                
                if (pinWord == "PIN") {
                    pinStream >> prob.pins[i].id >> prob.pins[i].x >> prob.pins[i].y;
                }
                else{
                    cerr << "Error: invalid input in PINS" << endl;
                    return {};
                }
            }
        } else if (word == "BLKS") {
            iss >> prob.numBlockages;
            prob.blockages.resize(prob.numBlockages);
            
            for (int i = 0; i < prob.numBlockages; i++) {
                getline(file, line);
                istringstream blkStream(line);
                string blkWord;
                blkStream >> blkWord;
                
                if (blkWord == "BLK") {
                    blkStream >> prob.blockages[i].id >> prob.blockages[i].xll >> prob.blockages[i].yll >> prob.blockages[i].xur >> prob.blockages[i].yur;
                }
                else{
                    cerr << "Error: invalid input in BLKS" << endl;
                    return {};
                }
            }
        }
    }
    return prob;
}
