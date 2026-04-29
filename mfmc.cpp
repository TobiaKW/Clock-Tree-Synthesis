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
    vector<assignmentEdge> assignmentEdges;
    // (cost, pin, tap) set cost as first element for easier sorting
    for (int p = 0; p < prob.numPins; p++) {
        for (int t = 0; t < prob.numTaps; t++) {
            int cost = manhattanDistance(prob.pins[p], prob.taps[t]);
            int dist_to_center = abs(prob.taps[t].x - prob.GRID_SIZE/2) + abs(prob.taps[t].y - prob.GRID_SIZE/2)
                + abs(prob.pins[p].x - prob.GRID_SIZE/2) + abs(prob.pins[p].y - prob.GRID_SIZE/2);
            assignmentEdges.push_back({cost, dist_to_center, p, t});
        }
    }


    // Sort edges by cost
    sort(assignmentEdges.begin(), assignmentEdges.end());

    vector<int> tap_load(prob.numTaps, 0); // track load on each tap
    

    //assign pins to taps greedily based on sorted edges, if all pins are assigned, break the loop
    for (int i = 0; i < assignmentEdges.size() && assignment.size() < prob.numPins; i++) {
        int pin_id = assignmentEdges[i].pin;
        int tap_id = assignmentEdges[i].tap;

        // Check if this pin is already assigned
        if (assignment.find(pin_id) != assignment.end()) {
            continue; // already assigned
        }

        // For pins with same cost, look ahead and pick the tap with lowest load
        int current_cost = assignmentEdges[i].cost;
        int best_tap = -1;
        int min_load = prob.MAX_LOAD + 1;
        

        //new approach: if same cost, pick the tap with lowest load
        // Scan ahead while cost is the same
        for (int j = i; j < assignmentEdges.size() && assignmentEdges[j].cost == current_cost; j++) {
            if (assignmentEdges[j].pin == pin_id) {  // only consider edges for this pin
                int candidate_tap = assignmentEdges[j].tap;
                if (tap_load[candidate_tap] < min_load) {
                    min_load = tap_load[candidate_tap];
                    best_tap = candidate_tap;
                }
            }
        }
        
        // Assign to best tap if it has capacity
        if (best_tap != -1 && tap_load[best_tap] < prob.MAX_LOAD) {
            assignment[pin_id] = best_tap;
            tap_load[best_tap]++;
        }
    }
    
    
    cout << "Assignment: " << endl;
    for (const auto& pair : assignment) {
        cout << "Pin " << pair.first << " assigned to tap " << pair.second << endl;
    }
    return assignment;
}

int MFMC::manhattanDistance(const Pin& pin, const Tap& tap) const {
    return abs(pin.x - tap.x) + abs(pin.y - tap.y);
}

