#include <iostream>
#include <fstream>
#include <random>
#include <chrono>
#include <queue>
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
    vector<double> cost_discount(prob.numTaps, 1.0);
    MFMC mfmc;
    map<int, int> assignment = mfmc.assignPinsToTaps(prob, cost_discount);
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
    for (int retry = 0; retry < 3; retry++) {
        auto pinTapDist = [&](int pinId, int tapId) {
            return abs(prob.pins[pinId].x - prob.taps[tapId].x) +
                   abs(prob.pins[pinId].y - prob.taps[tapId].y);
        };
        Grid local_grid = grid;
        bool skip_retry = false;
        map<int, Tree> local_trees;  // Trees for this retry

        if (retry == 0) {//far-first pin order
            for (auto &tap_pair : pins_per_tap) {
                sort(tap_pair.second.begin(), tap_pair.second.end(), [&](int a, int b) {
                    return pinTapDist(a, tap_pair.first) > pinTapDist(b, tap_pair.first);
                });
            }
        }
        else if (retry == 1) {//near-first pin order
            for (auto &tap_pair : pins_per_tap) {
                sort(tap_pair.second.begin(), tap_pair.second.end(), [&](int a, int b) {
                    return pinTapDist(a, tap_pair.first) < pinTapDist(b, tap_pair.first);
                });
            }
        }
        else if (retry >= 2) {//random shuffle pin order
            for (auto &tap_pair : pins_per_tap) {
                shuffle(tap_pair.second.begin(), tap_pair.second.end(), engine);
            }
        }
        

        vector<int> tap_load(prob.numTaps, 0);
        for (const auto& pair : assignment) {
            int tap_id = pair.second;
            tap_load[tap_id]++;
        }
        vector<int> tap_order;
        for (int i = 0; i < prob.numTaps; i++) {
            tap_order.push_back(i);
        }
        if (retry == 0) {//descending tap load order
            sort(tap_order.begin(), tap_order.end(), [&](int a, int b) {
                return tap_load[a] > tap_load[b];
            });
        }
        else if (retry == 1) {//ascending tap load order
            sort(tap_order.begin(), tap_order.end(), [&](int a, int b) {
                return tap_load[a] < tap_load[b];
            });
        }
        else if (retry >= 2) {//random shuffle tap order
            shuffle(tap_order.begin(), tap_order.end(), engine);
        }

        for (int tap_id : tap_order) {
            auto itPinList = pins_per_tap.find(tap_id);
            if (itPinList == pins_per_tap.end()) {
                continue;
            }
            const vector<int>& pin_ids = itPinList->second;
            
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
                
                // Keep compatibility with existing tree bookkeeping.
                int delay = path.size() - 1;
                tree.setDelay(pin_id, delay);
            }

            local_trees[tap_id] = tree;  // Save tree for this tap
        }
        
        if (skip_retry) continue;

        // Eval.py-equivalent cost calculation:
        // 1) length = unique unit edges per tap (sum over taps),
        // 2) delay = BFS arrival from each tap over used edges.
        const int INF = 1e9;
        int local_max_delay = 0;
        int local_min_delay = INF;
        int local_total_wirelength = 0;

        for (int t = 0; t < prob.numTaps; ++t) {
            vector<vector<vector<int>>> used(
                2, vector<vector<int>>(prob.GRID_SIZE, vector<int>(prob.GRID_SIZE, 0)));

            auto itTree = local_trees.find(t);
            if (itTree != local_trees.end()) {
                const auto edges = itTree->second.getTreeEdges();
                for (const auto& edge : edges) {
                    int x1 = edge.first.x, y1 = edge.first.y;
                    int x2 = edge.second.x, y2 = edge.second.y;

                    if (x1 == x2) {
                        int x = x1;
                        int l = min(y1, y2), h = max(y1, y2);
                        for (int y = l; y < h; ++y) {
                            if (used[1][x][y] == 0) {
                                used[1][x][y] = 1;
                                local_total_wirelength++;
                            }
                        }
                    } else if (y1 == y2) {
                        int y = y1;
                        int l = min(x1, x2), h = max(x1, x2);
                        for (int x = l; x < h; ++x) {
                            if (used[0][x][y] == 0) {
                                used[0][x][y] = 1;
                                local_total_wirelength++;
                            }
                        }
                    }
                }
            }

            vector<vector<int>> arrival(prob.GRID_SIZE, vector<int>(prob.GRID_SIZE, INF));
            queue<pair<int, int>> q;
            int tx = prob.taps[t].x, ty = prob.taps[t].y;
            arrival[tx][ty] = 0;
            q.push({tx, ty});

            while (!q.empty()) {
                pair<int, int> front = q.front();
                int x = front.first;
                int y = front.second;
                q.pop();
                int cur = arrival[x][y];

                if (x > 0 && used[0][x - 1][y] && cur + 1 < arrival[x - 1][y]) {
                    arrival[x - 1][y] = cur + 1;
                    q.push({x - 1, y});
                }
                if (x < prob.GRID_SIZE - 1 && used[0][x][y] && cur + 1 < arrival[x + 1][y]) {
                    arrival[x + 1][y] = cur + 1;
                    q.push({x + 1, y});
                }
                if (y > 0 && used[1][x][y - 1] && cur + 1 < arrival[x][y - 1]) {
                    arrival[x][y - 1] = cur + 1;
                    q.push({x, y - 1});
                }
                if (y < prob.GRID_SIZE - 1 && used[1][x][y] && cur + 1 < arrival[x][y + 1]) {
                    arrival[x][y + 1] = cur + 1;
                    q.push({x, y + 1});
                }
            }

            auto itPins = pins_per_tap.find(t);
            if (itPins == pins_per_tap.end()) {
                continue;
            }
            for (int pin_id : itPins->second) {
                int px = prob.pins[pin_id].x;
                int py = prob.pins[pin_id].y;
                if (arrival[px][py] >= INF) {
                    skip_retry = true;
                    break;
                }
                local_max_delay = max(local_max_delay, arrival[px][py]);
                local_min_delay = min(local_min_delay, arrival[px][py]);
            }
            if (skip_retry) break;
        }
        if (skip_retry) continue;

        if (local_min_delay == INF) local_min_delay = 0;
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
