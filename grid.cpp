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

bool Grid::canUse(Point p1, Point p2)  {
    Edge& e = getEdge(p1, p2);
    return e.legal && e.usage < CAPACITY;
}