#include "mfmc.hpp"
#include <iostream>
#include <algorithm>
#include <climits>
#include <cmath>

using namespace std;
// Pin-to-tap assignment.
// Uses successive shortest paths algorithm (Greedy).


map<int, int> MFMC::assignPinsGreedy(const Problem& prob, const vector<double>& cost_discount) const {
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
            assignmentEdges.push_back({cost, p, t});
        }
    }

    // apply cost discount (double -> int with stable rounding)
    for (int i = 0; i < assignmentEdges.size(); i++) {
        double w = 1.0;
        if (assignmentEdges[i].tap >= 0 && assignmentEdges[i].tap < (int)cost_discount.size()) {
            w = cost_discount[assignmentEdges[i].tap];
        }
        assignmentEdges[i].cost = max(1, (int)lround(assignmentEdges[i].cost * w));
    }


    // Sort edges by cost
    sort(assignmentEdges.begin(), assignmentEdges.end());

    vector<int> tap_load(prob.numTaps, 0); // track load on each tap
    

    //assign pins to taps greedily based on sorted edges, if all pins are assigned, break the loop
    for (int i = 0; i < assignmentEdges.size() && assignment.size() < prob.numPins; i++) {
        int pin_id = assignmentEdges[i].pin;
        // Check if this pin is already assigned
        if (assignment.find(pin_id) != assignment.end()) {
            continue; // already assigned
        }

        // For pins with same cost, prefer lower current tap load.
        int current_cost = assignmentEdges[i].cost;
        int best_tap = -1;
        int min_load = INT_MAX;
        

        //new approach: if same cost, pick the tap with lowest load
        // Scan ahead while cost is the same
        for (int j = i; j < assignmentEdges.size() && assignmentEdges[j].cost == current_cost; j++) {
            if (assignmentEdges[j].pin == pin_id) {  // if different tap but same cost same pin, check load
                int candidate_tap = assignmentEdges[j].tap;
                if (tap_load[candidate_tap] >= prob.MAX_LOAD) {
                    continue;
                }

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
    
    return assignment;
}

//helper function to assign pins to taps with cost discount
map<int, int> MFMC::assignPinsToTaps(const Problem& prob, const vector<double>& cost_discount) {
    map<int, int> assignment = assignPinsGreedy(prob, cost_discount);
    assignment = reassign(assignment, prob);
    return assignment;
}


map<int, int> MFMC::reassign(const map<int, int>& assignment, const Problem& prob) {
    map<int, int> current_assignment = assignment;
    vector<double> cost_discount(prob.numTaps, 1.0);
    for (int iter = 0; iter < 10; iter++) {
        int priorityTap = detectSplitTapCluster(current_assignment, prob);
        if (priorityTap < 0) {
            return current_assignment; // until no significant outlier tap detected
        }
        cout << "Priority Tap: " << priorityTap << endl;
        cost_discount[priorityTap] = max(cost_discount[priorityTap] - 0.1, 0.5);//max discount is 0.5 to avoid stealing too many pins from other taps
        map<int, int> new_assignment = assignPinsGreedy(prob, cost_discount);      
        if (new_assignment == current_assignment) {
            return current_assignment;
        }
        current_assignment = new_assignment;
    }
    return current_assignment;
}

int MFMC::detectSplitTapCluster(const map<int, int>& assignment, const Problem& prob) const {
    if (prob.numTaps <= 0 || assignment.empty()) {
        return -1;
    }

    vector<int> max_distance_to_taps(prob.numTaps, 0);
    for (const auto& pair : assignment) {
        int pin_id = pair.first;
        int tap_id = pair.second;
        if (tap_id < 0 || tap_id >= prob.numTaps) continue;
        int dist = manhattanDistance(prob.pins[pin_id], prob.taps[tap_id]);
        max_distance_to_taps[tap_id] = max(max_distance_to_taps[tap_id], dist);
    }

    int worst_tap = 0;
    int worst_dist = max_distance_to_taps[0];
    double sum = 0.0;
    for (int t = 0; t < prob.numTaps; ++t) {
        sum += max_distance_to_taps[t];
        if (max_distance_to_taps[t] > worst_dist) {
            worst_dist = max_distance_to_taps[t];
            worst_tap = t;
        }
    }

    double avg = sum / prob.numTaps;
    double sq_sum = 0.0;
    for (int t = 0; t < prob.numTaps; ++t) {
        double diff = max_distance_to_taps[t] - avg;
        sq_sum += diff * diff;
    }
    double stddev = sqrt(sq_sum / prob.numTaps);

    // Significant outlier gate: above average by both relative and absolute margin.
    const double kSigma = 1.4;//tunable
    const double minDelta = max(5.0, prob.GRID_SIZE/40.0);//tunable
    if (worst_dist >= avg + kSigma * stddev && (worst_dist - avg) >= minDelta) {
        return worst_tap;
    }
    return -1;
}

int MFMC::manhattanDistance(const Pin& pin, const Tap& tap) const {
    return abs(pin.x - tap.x) + abs(pin.y - tap.y);
}

