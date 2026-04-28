#include <iostream>
#include <fstream>
#include "types.hpp"
#include "parser.hpp"
#include "grid.hpp"
#include "mfmc.hpp"
#include "astar.hpp"

using namespace std;

int main(int argc, char* argv[]){

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

    // parse problem
    Problem prob = ParseProblem(inFile);

    // initialize grid
    Grid grid;
    grid.gridInit(prob);

    // assign pins to taps
    MFMC mfmc;
    map<int, int> assignment = mfmc.assignPinsToTaps(prob);
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
    
    // Track global delays for final cost
    int global_max_delay = 0;
    int global_min_delay = INT_MAX;
    int total_wirelength = 0;
    int best_cost = INT_MAX;
    
    // Route each tap's pins to its own tree
    for (int retry = 0; retry < 5; retry++) {
        for (int i = 0; i < pins_per_tap.size(); i++) {
            shuffle(pins_per_tap.begin(), pins_per_tap.end(), random_device());
        }
        
        for (const auto& tap_pair : pins_per_tap) {
            int tap_id = tap_pair.first;
            const vector<int>& pin_ids = tap_pair.second;
            
            Tree tree;
            tree.addPoint({prob.taps[tap_id].x, prob.taps[tap_id].y});  // Initialize with tap
            
            for (int pin_id : pin_ids) {
                vector<Point> path = astar.routePin(prob.pins[pin_id], tree, grid, prob);
                if (path.empty()) {
                    cerr << "Error: could not route pin " << pin_id << " to tap " << tap_id << "\n";
                    return 1;
                }
                
                // Add path to tree
                for (int i = 0; i < path.size() - 1; i++) {
                    tree.addPoint(path[i]);
                    tree.addEdge({path[i], path[i+1]});
                    grid.updateUsage(path[i], path[i+1]);
                }
                tree.addPoint(path.back());
                
                // Track delay for this pin
                int delay = path.size() - 1;
                tree.setDelay(pin_id, delay);
                
                cout << "Pin " << pin_id << " routed to tap " << tap_id << " with delay " << delay << endl;
            }
            
            // Report skew for this tap
            int tap_skew = tree.getSkew();
            cout << "Tap " << tap_id << " skew: " << tap_skew << endl;
            
            // Track global delays and wirelength
            for (const auto& pair : pins_per_tap[tap_id]) {
                int pin_delay = tree.getDelay(pair);
                if (pin_delay != -1) {
                    global_max_delay = max(global_max_delay, pin_delay);
                    global_min_delay = min(global_min_delay, pin_delay);
                }
            }
            total_wirelength += tree.getWirelength();
            cout << endl;
        }
        
        // Calculate final cost
        if (global_min_delay == INT_MAX) global_min_delay = 0;
        int global_skew = global_max_delay - global_min_delay;
        int cost = global_skew * prob.numTaps + total_wirelength;
        if (cost < best_cost) {
            best_cost = cost;
        }
        
        cout << "=== Results for retry " << retry << " ===" << endl;
        cout << "Global skew: " << global_skew << endl;
        cout << "Total wirelength: " << total_wirelength << endl;
        cout << "Final cost: " << cost << endl;
    }
    
    cout << "=== Best cost: " << best_cost << " ===" << endl;
    return 0;
}
