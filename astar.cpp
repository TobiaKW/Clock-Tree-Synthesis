#include "astar.hpp"
#include "grid.hpp"
#include <iostream>
#include <algorithm>
#include <cmath>
#include <utility>


using namespace std;

vector<Point> AStar::routePin(const Pin& pin, const set<Point>& tree,
                               const Grid& grid, const Problem& prob) {
    // TODO: Implement A* search
    // 1. Initialize open_set (priority queue), closed_set, g_cost, parent
    // a queue of int-point pair compared with (greater<>) and stored the sorted queue in the vector<pair> called open_set
    // int is the f-value, Point is the corresponding node
    // pin --g--> node --h-->tree point, f=g+h
    priority_queue<pair<int, Point>, vector<pair<int, Point>>, greater<pair<int, Point>>> open_set;
    set<Point> closed_set;
    map<Point, Point> parent;
    map<Point, int> g_cost;

    // 2. Start from pin.location, goal = closest tree point
    Point start = {pin.x, pin.y};
    int h_value = heuristic(start, tree);
    int f_value = h_value;//init
    g_cost.insert({start, 0});
    open_set.push(make_pair(0, start));

    // 3. While open_set not empty:
    //    - Pop lowest f-value node
    //    - If at goal, reconstruct and return path
    //    - For each neighbor:
    //      - Check legality (grid.canUse) and capacity
    //      - Calculate g_new = g_cost[current] + 1
    //      - Calculate h = heuristic(neighbor, tree)
    //      - If better path found, update and push to open_set
    while (!open_set.empty()) {
        auto top_entry = open_set.top();
        int f = top_entry.first;
        Point current = top_entry.second;
        open_set.pop();
        if(tree.count(current)) { // if current is in the tree >> path found
            //recontructpath
        }else{
            for (const Point& neighbor : getNeighbors(current)) {
                if(neighbor.x < 0 || neighbor.x >= prob.GRID_SIZE || neighbor.y < 0 || neighbor.y >= prob.GRID_SIZE || !grid.canUse(current, neighbor)) {
                    continue;
                }
                g_cost.insert({neighbor, g_cost[current] + 1});
                int h_new = heuristic(neighbor, tree);
                int f_new = g_cost[neighbor] + h_new;
                if(f_new < f) {
                    open_set.push(make_pair(f_new, neighbor));
                    parent[neighbor] = current;
                }
            }
        }
    }
    // 4. Return empty vector if no path found
    
    return {};  // placeholder
}

int AStar::manhattan(const Point& from, const Point& to) const {
    return abs(from.x - to.x) + abs(from.y - to.y);
}

int AStar::heuristic(const Point& from, const set<Point>& tree) const {
    int min_distance = INT_MAX;
    for (const Point& tree_point : tree) {
        int distance = manhattan(from, tree_point);
        if (distance < min_distance) {
            min_distance = distance;
        }
    }
    return min_distance;
}

vector<Point> AStar::reconstructPath(const map<Point, Point>& parent,
                                     const Point& start, const Point& goal) const {
    // TODO: Follow parent pointers from goal back to start
    // Return path in order: start -> ... -> goal
    
    return {};  // placeholder
}

vector<Point> AStar::getNeighbors(const Point& p) const {
    vector<Point> neighbors;
    neighbors.push_back({p.x+1, p.y});
    neighbors.push_back({p.x-1, p.y});
    neighbors.push_back({p.x, p.y+1});
    neighbors.push_back({p.x, p.y-1});
    return neighbors;
}
