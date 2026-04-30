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
    for (int iter = 0; iter < 8; iter++) {
        cout << "Iteration " << iter << endl;
        map<int, int> new_assignment = worstPinSwap(assignment, prob);
        if (new_assignment == assignment) {
            break;
        }
        assignment = new_assignment;
    }
    
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

map<int, int> MFMC::worstPinSwap(const map<int, int>& assignment, const Problem& prob) const {
    map<int, int> current_assignment = assignment;
    if (prob.numTaps <= 1 || prob.numPins <= 0 || current_assignment.empty()) {//robustness
        return current_assignment;
    }

    struct SwapCandidate {
        int pin_id;
        int from_tap;
        int to_tap;
        int gain;
    };
    vector<SwapCandidate> swap_candidates;
    swap_candidates.reserve(current_assignment.size() * 3);


    // Current per-tap pin counts and max assigned distance.
    vector<int> tap_load(prob.numTaps, 0);
    vector<int> tap_max_dist(prob.numTaps, 0);
    for (const auto& kv : current_assignment) {
        int pin_id = kv.first;
        int tap_id = kv.second;
        if (tap_id < 0 || tap_id >= prob.numTaps || pin_id < 0 || pin_id >= prob.numPins) continue;
        tap_load[tap_id]++;
        int dist = manhattanDistance(prob.pins[pin_id], prob.taps[tap_id]);
        tap_max_dist[tap_id] = max(tap_max_dist[tap_id], dist);
    }

    // Rank taps by max distance and gate by significant outlier rule.
    vector<pair<int, int>> tapMaxDistance; //<tap_id, max_distance>
    tapMaxDistance.reserve(prob.numTaps);
    double sum = 0.0;
    int active_taps = 0;
    for (int t = 0; t < prob.numTaps; ++t) {
        if (tap_load[t] == 0) continue;
        tapMaxDistance.push_back(make_pair(t, tap_max_dist[t]));
        sum += tap_max_dist[t];
        active_taps++;
    }
    if (active_taps == 0) return current_assignment;

    double mu = sum / active_taps;
    double sq_sum = 0.0;
    for (int i = 0; i < (int)tapMaxDistance.size(); ++i) {
        double diff = tapMaxDistance[i].second - mu;
        sq_sum += diff * diff;
    }
    double sigma = sqrt(sq_sum / active_taps);
    const double kSigma = 0.5;
    const double minDelta = max(3.0, prob.GRID_SIZE / 40.0);

    sort(tapMaxDistance.begin(), tapMaxDistance.end(), [](const pair<int, int>& a, const pair<int, int>& b) {
        return a.second > b.second;
    });//decreasing order

    vector<char> isTopTap(prob.numTaps, 0);
    int topK = min(3, (int)tapMaxDistance.size());
    int selected = 0;
    for (int i = 0; i < (int)tapMaxDistance.size() && selected < topK; ++i) {
        int tap_id = tapMaxDistance[i].first;
        int d = tapMaxDistance[i].second;
        if (d >= mu + kSigma * sigma && (d - mu) >= minDelta) {//gate to ensure top 3 taps are outliers
            isTopTap[tap_id] = 1;
            selected++;
        }
    }
    // Fallback if no tap passes the gate.
    if (selected == 0) {
        for (int i = 0; i < topK; ++i) {
            isTopTap[tapMaxDistance[i].first] = 1;
        }
    }

    for (const auto& kv : current_assignment ) {
        int pin_id = kv.first;
        int tap_id = kv.second;
        if (!isTopTap[tap_id]) continue;
        if (tap_id < 0 || tap_id >= prob.numTaps || pin_id < 0 || pin_id >= prob.numPins) continue;

        int d_assign = manhattanDistance(prob.pins[pin_id], prob.taps[tap_id]);
        vector<pair<int, int>> betterAlt; //<to_tap, distance>

        for (int t = 0; t < prob.numTaps; t++) {
            if (t == tap_id) continue;
            int d = manhattanDistance(prob.pins[pin_id], prob.taps[t]);
            if (d < d_assign) {
                betterAlt.push_back({t, d});
            }
        }

        sort(betterAlt.begin(), betterAlt.end(), [](const pair<int, int>& a, const pair<int, int>& b) {
            return a.second < b.second;
        });//sort by distance increasing

        for (int i = 0; i < min(3, (int)betterAlt.size()); i++) {
            int to_tap = betterAlt[i].first;
            int gain = d_assign - betterAlt[i].second;
            if (gain > 0) {
                swap_candidates.push_back({pin_id, tap_id, to_tap, gain});
            }
        }
    }

    sort(swap_candidates.begin(), swap_candidates.end(),
         [](const SwapCandidate& a, const SwapCandidate& b) {
             if (a.gain != b.gain) return a.gain > b.gain;
             return a.pin_id < b.pin_id;
         });

    // best partner 2-swap:
    // for candidate p: from_tap -> to_tap, find q currently in to_tap that maximizes
    // gain = [d(p,from)+d(q,to)] - [d(p,to)+d(q,from)].
    for (int i = 0; i < min(200, (int)swap_candidates.size()); i++) {
        //if (swapped[i]) continue;

        int p = swap_candidates[i].pin_id;
        int from_tap = swap_candidates[i].from_tap;
        int to_tap = swap_candidates[i].to_tap;
        if (from_tap < 0 || from_tap >= prob.numTaps || to_tap < 0 || to_tap >= prob.numTaps) continue;//robustness
        if (current_assignment[p] != from_tap) continue; // stale candidate after previous moves/swaps

        int best_partner_pin = -1;
        int best_gain = 0;

        for (const auto& kv : current_assignment) {
            int q = kv.first;
            int q_tap = kv.second;
            if (q_tap != to_tap || q == p) continue;

            int cur_cost = manhattanDistance(prob.pins[p], prob.taps[from_tap]) +
                           manhattanDistance(prob.pins[q], prob.taps[to_tap]);
            int new_cost = manhattanDistance(prob.pins[p], prob.taps[to_tap]) +
                           manhattanDistance(prob.pins[q], prob.taps[from_tap]);
            int gain = cur_cost - new_cost;
            if (gain > best_gain) {
                best_gain = gain;
                best_partner_pin = q;
            }
        }

        if (best_partner_pin != -1 && best_gain > 0) {
            current_assignment[p] = to_tap;
            current_assignment[best_partner_pin] = from_tap;
            cout << "best-partner swap: " << p << " (" << from_tap << "->" << to_tap
                 << "), partner " << best_partner_pin << " (" << to_tap << "->" << from_tap
                 << "), gain " << best_gain << endl;
        }
    }
    // Keep this function behavior-preserving for now.
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
    const double kSigma = 1.3;//tunable
    const double minDelta = max(5.0, prob.GRID_SIZE/40.0);//tunable
    if (worst_dist >= avg + kSigma * stddev && (worst_dist - avg) >= minDelta) {
        return worst_tap;
    }
    return -1;
}

int MFMC::manhattanDistance(const Pin& pin, const Tap& tap) const {
    return abs(pin.x - tap.x) + abs(pin.y - tap.y);
}

