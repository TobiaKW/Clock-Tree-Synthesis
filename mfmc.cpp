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

    vector<int> tap_load(prob.numTaps, 0); // track load on each tap
    

    //assign pins to taps greedily based on sorted edges, if all pins are assigned, break the loop
    for (int i = 0; i < edges.size() && assignment.size() < prob.numPins; i++) {
        int pin_id = edges[i].pin;
        int tap_id = edges[i].tap;

        // Check if this pin is already assigned
        if (assignment.find(pin_id) != assignment.end()) {
            continue; // already assigned
        }

        // Check if tap has capacity
        int current_load = 0;
        for (const auto& pair : assignment) {
            if (pair.second == tap_id) {
                current_load++;
            }
        }
        if (current_load < prob.MAX_LOAD) {
            assignment[pin_id] = tap_id; // assign pin to tap
        }

    }

    return assignment;
}

int MFMC::manhattanDistance(const Pin& pin, const Tap& tap) const {
    return abs(pin.x - tap.x) + abs(pin.y - tap.y);
}
