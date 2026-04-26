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

void Tree::removePoint(Point p) {
    tree_points.erase(p);
}  

void Tree::removeEdge(pair<Point, Point> edge) {
    tree_edges.erase(edge);
}

set<pair<Point, Point>> Tree::getTreeEdges() {
    return tree_edges;
}

set<Point> Tree::getTreePoints() {
    return tree_points;
}