#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>

using namespace std;

class Problem;
Problem ParseProblem(const char* filename);

/**==========data structures==========**/
struct Tap {
    int id;
    int x;
    int y;
};

struct Pin {
    int id;
    int x;
    int y;
};

struct Blockage {
    int id;
    int xll;//lower left
    int yll;
    int xur;//upper right
    int yur;
};

class Problem {
public:
    int MAX_LOAD;
    int GRID_SIZE;
    int CAPACITY;
    int numTaps;
    int numPins;
    int numBlockages;
    vector<Tap> taps;
    vector<Pin> pins;
    vector<Blockage> blockages;
};
/**==========end of data structures==========**/


int main(int argc, char* argv[]){

    // check arguments
    if (argc != 3) {
        cerr << "usage: xor <in_file> <out_file>\n";
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

}

/*================= Parser ========================*/
Problem ParseProblem(const char* filename) {
    ifstream file(filename);
    if (!file) {
        cerr << "Error: could not open file " << filename << "\n";
        return {};
    }

    Problem prob;
    string line, word;

    while (getline(file, line)) { //get one full line
        if (line.empty()) {
            continue;
        }
        istringstream iss(line); //divide the full string by space
        iss >> word; // get the first word and check, next iss >> will get the next word
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
                istringstream tapStream(line);//create a new stream for the every line
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
/*================= end of Parser =================*/

