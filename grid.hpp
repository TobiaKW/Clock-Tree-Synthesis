#ifndef GRID_HPP
#define GRID_HPP

#include <utility>
#include <map>

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
public:
    int GRID_SIZE;
    int CAPACITY;

    /* dictionary called "edges", we use a pair of points as the key to get the edge*/
    map<pair<Point, Point>, Edge> edges;

    pair<Point, Point> canonicalEdge(Point p1, Point p2);
    Edge& getEdge(Point p1, Point p2);  // returns canonical edge REFERENCE
    bool canUse(Point p1, Point p2);    // check legal && usage < CAPACITY
};

#endif