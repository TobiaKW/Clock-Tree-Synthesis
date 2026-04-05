#include "mfmc.hpp"
#include <iostream>
#include <algorithm>

using namespace std;
// Pin-to-tap assignment.
// Uses successive shortest paths algorithm (Greedy).


map<int, int> MFMC::assignPinsToTaps(const Problem& prob) {
    // Check feasibility: total pins <= total capacity
    if (prob.numPins > prob.numTaps * prob.MAX_LOAD) {
        cerr << "Error: infeasible - too many pins for available tap capacity\n";
        return {};
    }
    
    map<int, int> assignment;  // pin_id -> tap_id
    
    // Pre-compute all edges
    vector<Edge> edges;
    // (cost, pin, tap) set cost as first element for easier sorting
    for (int p = 0; p < prob.numPins; p++) {
        for (int t = 0; t < prob.numTaps; t++) {
            int cost = manhattanDistance(prob.pins[p], prob.taps[t]);
            int dist_to_center = abs(prob.taps[t].x - prob.GRID_SIZE/2) + abs(prob.taps[t].y - prob.GRID_SIZE/2)
                + abs(prob.pins[p].x - prob.GRID_SIZE/2) + abs(prob.pins[p].y - prob.GRID_SIZE/2);
            edges.push_back({cost, dist_to_center, p, t});
        }
    }


    // Sort edges by cost
    sort(edges.begin(), edges.end());

    //TODO: assignment



    return assignment;
}

int MFMC::manhattanDistance(const Pin& pin, const Tap& tap) const {
    return abs(pin.x - tap.x) + abs(pin.y - tap.y);
}

pair<int, map<int, int>> MFMC::findAugmentingPath(
    const Problem& prob,
    const map<int, int>& currentAssignment
) const {


    
    cerr << "TODO: findAugmentingPath not yet implemented\n";
    return {-1, {}};
}
