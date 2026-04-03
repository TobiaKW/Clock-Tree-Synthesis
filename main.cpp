#include <iostream>
#include <fstream>
#include <vector>
#include <string>

using namespace std;

vector<string> read_file(const char* filename);

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

    vector<string> lines = read_file(inFile);
    parsing(lines);
    

}


vector<string> read_file(const char* filename) {
    ifstream file(filename);
    if (!file) {
        cerr << "Error: could not open file " << filename << "\n";
        return {};
    }

    vector<string> lines;
    string line;

    while (getline(file, line)) { //get one full line
        lines.push_back(line); //put the line into the string vector
    }
    return lines;
}