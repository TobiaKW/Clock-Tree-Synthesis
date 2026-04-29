#include <iostream>
#include <fstream>
#include <random>
#include <chrono>
#include "types.hpp"
#include "parser.hpp"
#include "grid.hpp"
#include "mfmc.hpp"
#include "astar.hpp"

using namespace std;

int main(int argc, char* argv[]){
    auto start_total = chrono::high_resolution_clock::now();

    // check arguments
    if (argc != 3) {
        cerr << "usage: cts <in_file> <out_file>\n";
        return 1;
    }
    const char* inFile = argv[1];
    const char* outFile = argv[2];
    ofstream out(outFile);
    if (!out) {
        cerr << "Error: could not open output file " << outFile << "\n";
    }

    auto start = chrono::high_resolution_clock::now();
    // parse problem
    Problem prob = ParseProblem(inFile);

    // initialize grid
    Grid grid;
    grid.gridInit(prob);
    auto end = chrono::high_resolution_clock::now();
    cerr << "[PROFILE] Parse + Grid init: " << chrono::duration<double>(end - start).count() << "s\n";

    // assign pins to taps
    start = chrono::high_resolution_clock::now();
    MFMC mfmc;
    map<int, int> assignment = mfmc.assignPinsToTaps(prob);
    end = chrono::high_resolution_clock::now();
    cerr << "[PROFILE] MFMC assignment: " << chrono::duration<double>(end - start).count() << "s\n";

    //output: matching result

    // route pins to taps (per-tap trees)
    AStar astar;
    
    // Group pins by tap
    map<int, vector<int>> pins_per_tap; // [tap_id] -> [vector of pin_id]
    for (const auto& pair : assignment) {
        int pin_id = pair.first;
        int tap_id = pair.second;
        pins_per_tap[tap_id].push_back(pin_id);
    }
    
    // Track best solution across retries
    int best_cost = INT_MAX;
    int best_global_max_delay = 0;
    int best_global_min_delay = INT_MAX;
    int best_total_wirelength = 0;
    Grid best_grid = grid;
    map<int, Tree> best_trees;  // Store best tree for each tap
    
    // Random engine for shuffling
    default_random_engine engine{random_device()()};

    // Route each tap's pins to its own tree
    auto start_routing = chrono::high_resolution_clock::now();
    for (int retry = 0; retry < 2; retry++) {

        int local_max_delay = 0;
        int local_min_delay = INT_MAX;
        int local_total_wirelength = 0;
        Grid local_grid = grid;
        bool skip_retry = false;
        map<int, Tree> local_trees;  // Trees for this retry

        //level 1 backtrack: shuffle pin order
        for (auto &tap_pair : pins_per_tap) {
            shuffle(tap_pair.second.begin(), tap_pair.second.end(), engine);
        }
        
        for (const auto& tap_pair : pins_per_tap) {
            int tap_id = tap_pair.first;
            const vector<int>& pin_ids = tap_pair.second;
            
            Tree tree;
            tree.addPoint({prob.taps[tap_id].x, prob.taps[tap_id].y});  // Initialize with tap
            
            for (int pin_id : pin_ids) {
                vector<Point> path = astar.routePin(prob.pins[pin_id], tree, local_grid, prob);
                if (path.empty()) {
                    cerr << "Error: could not route pin " << pin_id << " to tap " << tap_id << " (skipping retry)\n";
                    skip_retry = true;
                    break;
                }
                
                // Add path to tree
                for (int i = 0; i < path.size() - 1; i++) {
                    tree.addPoint(path[i]);
                    tree.addEdge({path[i], path[i+1]});
                    local_grid.updateUsage(path[i], path[i+1]);
                }
                tree.addPoint(path.back());
                
                // Track delay for this pin
                int delay = path.size() - 1;
                tree.setDelay(pin_id, delay);
                
                // cout << "Pin " << pin_id << " routed to tap " << tap_id << " with delay " << delay << endl;
            }
            
            // Report skew for this tap
            int tap_skew = tree.getSkew();
            // cout << "Tap " << tap_id << " skew: " << tap_skew << endl;
            
            // Track global delays and wirelength
            for (int pin_id : pins_per_tap[tap_id]) {
                int pin_delay = tree.getDelay(pin_id);
                if (pin_delay != -1) {
                    local_max_delay = max(local_max_delay, pin_delay);
                    local_min_delay = min(local_min_delay, pin_delay);
                }
            }
            local_total_wirelength += tree.getWirelength();
            // cout << endl;
            
            local_trees[tap_id] = tree;  // Save tree for this tap
        }
        
        if (skip_retry) continue;
        
        // Calculate final cost using GLOBAL skew (max/min across all pins)
        if (local_min_delay == INT_MAX) local_min_delay = 0;
        int global_skew = local_max_delay - local_min_delay;
        int cost = global_skew * prob.numTaps + local_total_wirelength;
        if (cost < best_cost) {
            best_cost = cost;
            best_global_max_delay = local_max_delay;
            best_global_min_delay = local_min_delay;
            best_total_wirelength = local_total_wirelength;
            best_grid = local_grid;
            best_trees = local_trees;  // Save best trees
        }
        
        cout << "=== Results for retry " << retry << " ===" << endl;
        cout << "Global skew: " << global_skew << endl;
        cout << "Local wirelength: " << local_total_wirelength << endl;
        cout << "Local cost: " << cost << endl;
    }
    auto end_routing = chrono::high_resolution_clock::now();
    cerr << "[PROFILE] A* routing (all retries): " << chrono::duration<double>(end_routing - start_routing).count() << "s\n";
    
    cout << "=== Best cost: " << best_cost << " ===" << endl;
    
    // Output best solution to file
    for (const auto& tap_pair : best_trees) {
        int tap_id = tap_pair.first;
        const Tree& tree = tap_pair.second;
        
        // Get pins assigned to this tap
        vector<int> pins = pins_per_tap[tap_id];
        
        out << "TAP " << tap_id << "\n";
        out << "PINS " << pins.size() << "\n";
        for (int pin_id : pins) {
            out << "PIN " << pin_id << "\n";
        }
        
        // Output edges
        auto edges = tree.getTreeEdges();
        out << "ROUTING " << edges.size() << "\n";
        for (const auto& edge : edges) {
            out << "EDGE " << edge.first.x << " " << edge.first.y << " " 
                << edge.second.x << " " << edge.second.y << "\n";
        }
    }
    
    out.close();
    auto end_total = chrono::high_resolution_clock::now();
    cerr << "[PROFILE] TOTAL TIME: " << chrono::duration<double>(end_total - start_total).count() << "s\n";
    return 0;

}
