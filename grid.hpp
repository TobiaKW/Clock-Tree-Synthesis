#ifndef GRID_HPP
#define GRID_HPP

#include "types.hpp"
#include <utility>
#include <map>
#include <set>

using namespace std;

struct Point {
    int x;
    int y;
    
    bool operator<(const Point& other) const {//compare point method
        if (x != other.x) {
            return x < other.x;
        }else{
            return y < other.y;
        }
    }
};

struct Edge {
    int usage;
    bool legal;
};

class Grid {
private:
    int GRID_SIZE;
    int CAPACITY;

    /* dictionary called "edges", we use a pair of points as the key to get the edge*/
    map<pair<Point, Point>, Edge> edges;

    pair<Point, Point> canonicalEdge(Point p1, Point p2) const;
    Edge& getEdge(Point p1, Point p2);  // returns canonical edge REFERENCE

public:
    bool canUse(Point p1, Point p2) const;    // check legal && usage < CAPACITY
    void gridInit(const Problem& prob);
};


class Tree{
private:
    set<Point> tree_points;
    set<pair<Point, Point>> tree_edges;
    //todo: skew etc
public:
    void addPoint(Point p);
    void addEdge(pair<Point, Point> edge);
    void removePoint(Point p);
    void removeEdge(pair<Point, Point> edge);
    set<pair<Point, Point>> getTree();
    set<Point> getTreePoints();
    set<pair<Point, Point>> getTreeEdges();
};

#endif