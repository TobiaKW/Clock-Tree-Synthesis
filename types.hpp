#ifndef TYPES_HPP
#define TYPES_HPP

#include <vector>

using namespace std;

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
    int xll;  // lower left
    int yll;
    int xur;  // upper right
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

#endif
