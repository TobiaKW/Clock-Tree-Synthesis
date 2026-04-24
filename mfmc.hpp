#ifndef MFMC_HPP
#define MFMC_HPP

#include "types.hpp"
#include <vector>
#include <map>

using namespace std;

struct assignmentEdge {
    int cost, dist_to_center, pin, tap;
    bool operator<(const assignmentEdge& other) const {
        if (cost != other.cost) return cost < other.cost;
        return dist_to_center < other.dist_to_center;  // closer to center first
    }
};


class MFMC {
public:
    map<int, int> assignPinsToTaps(const Problem& prob);

private:
    int manhattanDistance(const Pin& pin, const Tap& tap) const;
    
};

#endif
