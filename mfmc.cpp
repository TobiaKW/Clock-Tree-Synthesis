#include "mfmc.hpp"
#include <iostream>
#include <algorithm>
#include <climits>

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
            assignmentEdges.push_back({cost, p, t});
        }
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
    
    assignment = reassign(assignment, prob);
    /*cout << "Assignment: " << endl;
    for (const auto& pair : assignment) {
        cout << "Pin " << pair.first << " assigned to tap " << pair.second << endl;
    }
    */
    return assignment;
}


map<int, int> MFMC::reassign(const map<int, int>& assignment, const Problem& prob) {
    //Phase 1: inspect current assignment!
    map<int, int> current_assignment = assignment;
    vector<int> max_distance_to_taps(prob.numTaps, 0);
    vector<vector<int>> pins_per_tap(prob.numTaps);

    // Calculate per-tap load and max pin-to-tap Manhattan distance
    for (const auto& pair : current_assignment) {
        int pin_id = pair.first;
        int tap_id = pair.second;
        if (tap_id < 0 || tap_id >= prob.numTaps) continue;

        pins_per_tap[tap_id].push_back(pin_id);

        int dist = manhattanDistance(prob.pins[pin_id], prob.taps[tap_id]);
        if (dist > max_distance_to_taps[tap_id]) {
            max_distance_to_taps[tap_id] = dist;
        }
    }

    // Find taps with highest max delay first
    vector<pair<int, int>> tapRank; // {tap_id, maxDist}
    tapRank.reserve(prob.numTaps);
    for (int t = 0; t < prob.numTaps; ++t) {
        tapRank.push_back({t, max_distance_to_taps[t]});
    }
    sort(tapRank.begin(), tapRank.end(),
         [](auto& a, auto& b){ return a.second > b.second; });

    // Phase 2: redo assignment but prioritize taps with highest max delay!
    map<int, int> new_assignment;
    vector<int> new_tap_load(prob.numTaps, 0);
    vector<assignmentEdge> assignmentEdges;
    int priorityTap = tapRank.front().first;   // highest maxDist tap_id
    //int priorityVal = tapRank.front().second;  // its maxDist, shd be no use

    vector<pair<int, int>> dist_to_priorityTap;
    for (int p = 0; p < prob.numPins; p++) {
        dist_to_priorityTap.push_back({p, manhattanDistance(prob.pins[p], prob.taps[priorityTap])});
    }
    sort(dist_to_priorityTap.begin(), dist_to_priorityTap.end(),
         [](auto& a, auto& b){ return a.second < b.second; });

    vector<bool> assigned(prob.numPins, false);

    for (const auto& pair : dist_to_priorityTap) {
        int pin_id = pair.first;
        if (new_tap_load[priorityTap] >= prob.MAX_LOAD) break;

        /* needs to think think
        int best_other = INT_MAX;
        for (int t = 0; t < prob.numTaps; ++t) {
            if (t == priorityTap) continue;
            best_other = min(best_other, manhattanDistance(prob.pins[pin_id], prob.taps[t]));
        }
        if (pair.second > best_other + 2) continue; // margin=1, tunable
        */
        new_assignment[pin_id] = priorityTap;
        assigned[pin_id] = true;
        new_tap_load[priorityTap]++;
    }
    //Phase 2.5: assign the remainings
    //assign pins to taps greedily based on sorted edges, if all pins are assigned, break the loop
    for (int p = 0; p < prob.numPins; p++) {
        for (int t = 0; t < prob.numTaps; t++) {
            int cost = manhattanDistance(prob.pins[p], prob.taps[t]);
            assignmentEdges.push_back({cost, p, t});
        }
    }
    sort(assignmentEdges.begin(), assignmentEdges.end());

    for (int i = 0; i < assignmentEdges.size() && new_assignment.size() < prob.numPins; i++) {
        int pin_id = assignmentEdges[i].pin;
        // Check if this pin is already assigned
        if (new_assignment.find(pin_id) != new_assignment.end() || assigned[pin_id]) {
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
                if (new_tap_load[candidate_tap] >= prob.MAX_LOAD) {
                    continue;
                }

                if (new_tap_load[candidate_tap] < min_load) {
                    min_load = new_tap_load[candidate_tap];
                    best_tap = candidate_tap;
                }
            }
        }
        
        // Assign to best tap if it has capacity
        if (best_tap != -1 && new_tap_load[best_tap] < prob.MAX_LOAD) {
            new_assignment[pin_id] = best_tap;
            new_tap_load[best_tap]++;
        }
    }

    cout << "New Assignment generated successfully" << endl;
    
    auto proxyScore = [&](const map<int, int>& asg) -> pair<int, long long> {
        vector<int> tap_max(prob.numTaps, 0);
        long long total_manhattan = 0;

        // Penalize incomplete assignments heavily so they are never preferred.
        if ((int)asg.size() != prob.numPins) {
            return {INT_MAX / 2, LLONG_MAX / 2};
        }

        for (const auto& kv : asg) {
            int pin_id = kv.first;
            int tap_id = kv.second;
            if (tap_id < 0 || tap_id >= prob.numTaps) {
                return {INT_MAX / 2, LLONG_MAX / 2};
            }
            int d = manhattanDistance(prob.pins[pin_id], prob.taps[tap_id]);
            total_manhattan += d;
            tap_max[tap_id] = max(tap_max[tap_id], d);
        }

        int max_tap_dist = 0;
        for (int t = 0; t < prob.numTaps; ++t) {
            max_tap_dist = max(max_tap_dist, tap_max[t]);
        }
        return {max_tap_dist, total_manhattan};
    };

    pair<int, long long> base_score = proxyScore(current_assignment);
    pair<int, long long> new_score = proxyScore(new_assignment);

    if (new_score.first < base_score.first ||
        (new_score.first == base_score.first && new_score.second < base_score.second)) {
        cout << "New Assignment is better than current assignment" << endl;
        return new_assignment;
    }
    cout << "New Assignment is not better than current assignment" << endl;
    return current_assignment;
}

int MFMC::manhattanDistance(const Pin& pin, const Tap& tap) const {
    return abs(pin.x - tap.x) + abs(pin.y - tap.y);
}

