#ifndef ASTAR_HPP
#define ASTAR_HPP

#include "types.hpp"
#include "grid.hpp"
#include <vector>
#include <set>
#include <map>
#include <queue>


using namespace std;

class AStar {
public:
    // Route a single pin from its location to the existing tree
    // Returns path as vector of points (pin -> ... -> tree)
    // Returns empty vector if no path found
    vector<Point> routePin(const Pin& pin, const Tree& tree, 
                           const Grid& grid, const Problem& prob);

private:
    // Manhattan distance between two points
    int manhattan(const Point& from, const Point& to) const;
    
    // Manhattan distance from point to nearest point in tree
    int heuristic(const Point& from, const Tree& tree) const;
    
    // Reconstruct path from parent map
    vector<Point> reconstructPath(const map<Point, Point>& parent, 
                                  const Point& start, const Point& goal) const;
    
    // Get 4 neighbors (up, down, left, right)
    vector<Point> getNeighbors(const Point& p) const;
};

#endif
