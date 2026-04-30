#ifndef MFMC_HPP
#define MFMC_HPP

#include "types.hpp"
#include <vector>
#include <map>

using namespace std;

struct assignmentEdge {
    int cost, pin, tap;
    bool operator<(const assignmentEdge& other) const {
        if (cost != other.cost) return cost < other.cost;
        if (pin != other.pin) return pin < other.pin;
        return tap < other.tap;
    }
};


class MFMC {
public:
    map<int, int> assignPinsToTaps(const Problem& prob, const vector<double>& cost_discount);

private:
    map<int, int> assignPinsGreedy(const Problem& prob, const vector<double>& cost_discount) const;
    map<int, int> reassign(const map<int, int>& assignment, const Problem& prob);
    map<int, int> worstPinSwap(const map<int, int>& assignment, const Problem& prob) const;
    int detectSplitTapCluster(const map<int, int>& assignment, const Problem& prob) const;
    int manhattanDistance(const Pin& pin, const Tap& tap) const;
};

#endif
