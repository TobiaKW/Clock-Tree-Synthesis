#include "astar.hpp"
#include "grid.hpp"

#include <iostream>
#include <algorithm>
#include <cmath>
#include <climits>
#include <utility>


using namespace std;

vector<Point> AStar::routePin(const Pin& pin, const Tree& tree,
                               const Grid& grid, const Problem& prob) {
    priority_queue<pair<int, Point>, vector<pair<int, Point>>, greater<pair<int, Point>>> open_set;
    set<Point> closed_set;
    map<Point, Point> parent;
    map<Point, int> g_cost;
    set<Point> tree_points = tree.getTreePoints();
    set<pair<Point, Point>> tree_edges = tree.getTreeEdges();
    
    int closest_tree_dist = INT_MAX;  // Cache for heuristic

    // 2. Start from pin.location, goal = closest tree point
    Point start = {pin.x, pin.y};
    closest_tree_dist = heuristic(start, tree_points, closest_tree_dist);
    int f_value = closest_tree_dist; //initial g = 0,so f=h
    g_cost.insert({start, 0});
    open_set.push(make_pair(f_value, start));

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
        if(closed_set.count(current)) {
            continue;
        }
        if(tree_points.find(current) != tree_points.end() && 
           !(current.x == start.x && current.y == start.y)) {
            return reconstructPath(parent, start, current);
        } else {
            for (const Point& neighbor : getNeighbors(current)) {
                if(neighbor.x < 0 || neighbor.x >= prob.GRID_SIZE || neighbor.y < 0 || neighbor.y >= prob.GRID_SIZE || !grid.canUse(current, neighbor)) {
                    continue;
                }
                int g_new = g_cost.at(current) + 1;
                int h_new = heuristic(neighbor, tree_points, closest_tree_dist);
                int f_new = g_new + h_new;
                if (g_cost.find(neighbor) == g_cost.end() || g_new < g_cost[neighbor]) { //neighbor is new OR better than the previous one
                    g_cost[neighbor] = g_new;
                    open_set.push({f_new, neighbor});
                    parent[neighbor] = current;
                }
            }
            closed_set.insert(current);
        }
    }
    // 4. Return empty vector if no path found
    return {}; 
}

int AStar::manhattan(const Point& from, const Point& to) const {
    return abs(from.x - to.x) + abs(from.y - to.y);
}

int AStar::heuristic(const Point& from, const set<Point>& tree_points, int& closest_distance) const {
    int min_distance = INT_MAX;
    for (const Point& tree_point : tree_points) {
        int distance = manhattan(from, tree_point);
        if (distance < min_distance) {
            min_distance = distance;
        }
    }
    // Update cache if we found something closer
    if (min_distance < closest_distance) {
        closest_distance = min_distance;
    }
    return closest_distance;
}

vector<Point> AStar::reconstructPath(const map<Point, Point>& parent,
                                     const Point& start, const Point& goal) const {
    // TODO: Follow parent pointers from goal back to start
    // Return path in order: start -> ... -> goal
    vector<Point> path;
    Point current = goal;
    while (current.x != start.x || current.y != start.y) {
        path.push_back(current);
        current = parent.at(current);
    }
    path.push_back(start);
    reverse(path.begin(), path.end());
    return path;
}

vector<Point> AStar::getNeighbors(const Point& p) const {
    vector<Point> neighbors;
    neighbors.push_back({p.x+1, p.y});
    neighbors.push_back({p.x-1, p.y});
    neighbors.push_back({p.x, p.y+1});
    neighbors.push_back({p.x, p.y-1});
    return neighbors;
}
