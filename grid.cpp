#include "grid.hpp"

using namespace std;

pair<Point, Point> Grid::canonicalEdge(Point p1, Point p2) const {
    if (p2 < p1){
        return {p2, p1};
    }else{
        return {p1, p2};
    }
}

Edge& Grid::getEdge(Point p1, Point p2) {
    return edges[canonicalEdge(p1, p2)];
}

bool Grid::canUse(Point p1, Point p2) const {
    auto targetEdge = edges.find(canonicalEdge(p1, p2));
    //point to edges.end() means the edge is not found
    return targetEdge != edges.end() && targetEdge->second.legal && targetEdge->second.usage < CAPACITY;
}

void Grid::gridInit(const Problem& prob) {
    GRID_SIZE = prob.GRID_SIZE;
    CAPACITY = prob.CAPACITY;

    // initialize vertical edges
    for (int i = 0; i < prob.GRID_SIZE; i++) {
        for (int j = 0; j < prob.GRID_SIZE-1; j++) {
            Point p1 = {i, j};
            Point p2 = {i, j+1};
            edges[canonicalEdge(p1, p2)] = {0, true};
        }
    }
    // initialize horizontal edges
    for (int i = 0; i < prob.GRID_SIZE-1; i++) {
        for (int j = 0; j < prob.GRID_SIZE; j++) {
            Point p1 = {i, j};
            Point p2 = {i+1, j};
            edges[canonicalEdge(p1, p2)] = {0, true};
        }
    }
    // mark blockage interior edges as illegal
    for (const Blockage& blk : prob.blockages) {
        // Vertical edges inside blockage
        for (int i = blk.xll+1; i < blk.xur; i++) {
            for (int j = blk.yll; j < blk.yur; j++) {
                edges[canonicalEdge({i,j}, {i,j+1})].legal = false;
            }
        }
        // Horizontal edges inside blockage
        for (int i = blk.xll; i < blk.xur; i++) {
            for (int j = blk.yll+1; j < blk.yur; j++) {
                edges[canonicalEdge({i,j}, {i+1,j})].legal = false;
            }
        }
    }
}

void Tree::addPoint(Point p) {
    tree_points.insert(p);
}

void Tree::addEdge(pair<Point, Point> edge) {
    tree_edges.insert(edge);
}

void Grid::updateUsage(Point p1, Point p2) {
    getEdge(p1, p2).usage++;
}

void Tree::removePoint(Point p) {
    tree_points.erase(p);
}  

void Tree::removeEdge(pair<Point, Point> edge) {
    tree_edges.erase(edge);
}

set<pair<Point, Point>> Tree::getTreeEdges() const {
    return tree_edges;
}

set<Point> Tree::getTreePoints() const {
    return tree_points;
}

void Tree::setDelay(int pin_id, int delay_val) {
    pin_delay[pin_id] = delay_val;
}

int Tree::getDelay(int pin_id) const {
    auto it = pin_delay.find(pin_id);
    return it != pin_delay.end() ? it->second : -1; //if not found, return -1
}

int Tree::getSkew() const {
    if (pin_delay.empty()) return 0;
    
    int max_delay = 0;
    int min_delay = INT_MAX;
    for (const auto& pair : pin_delay) {
        int delay = pair.second;
        max_delay = max(max_delay, delay);
        min_delay = min(min_delay, delay);
    }
    //if min_delay is INT_MAX, it means no delay is set, return 0
    return (min_delay == INT_MAX) ? 0 : (max_delay - min_delay);
}

int Tree::getWirelength() const {
    return tree_edges.size();
}