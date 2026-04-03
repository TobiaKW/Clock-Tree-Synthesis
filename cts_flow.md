CTS algorithm (practical version)

Parse + preprocess
Read taps, pins, blocks, limits.
Build grid graph of unit edges.
Mark each unit edge as legal/illegal (illegal only if crossing block interior).
Initialize global edgeUsage = 0.

Pin-to-tap assignment
Assign each pin to one tap with MAX_LOAD constraint.
Baseline: nearest tap with remaining capacity.
Better: min-cost flow (cost = Manhattan distance).

Route each tap’s tree For one tap at a time:
Start tree with just the tap node.
While there are unconnected assigned pins:
Pick one pin.
Run BFS/A* (a*better) from that pin to any node already in this tree.
During search, allow only edges that are legal and edgeUsage < CAPACITY.
If found, add that path to the tree and increment edgeUsage. This naturally forms a rectilinear Steiner-like tree.

Conflict handling (important) If a pin cannot be connected: (no legal path)
Try different pin order (far-first / random / congestion-aware).
Rip-up and reroute this tap.
If still fails, reassign some pins between taps and reroute.

localretry: level1/level2 backtrack maybe 5times/2times
if localretry keeps failing >> globalretry (reshuffle tap/pin order)
after globalmaxretry, output the current best solution

Validate
Every pin connected exactly once.
Tap load <= MAX_LOAD.
No edge over CAPACITY.
No blockage-interior crossing.
No open/short violations.

Compute objective
Delay d_i: path length from tap to pin i along tree.
Skew = max(d_i) - min(d_i). (keep the clock spread acceptable)
Total wirelength = sum of routed edge lengths.
Cost: [ c = (\max d_i - \min d_i)\times numTaps + \sum TreeLength(TAP_k) ] /given in spec

Improve (optional loop)
Swap a few pins between taps, reroute locally, keep only better solutions.
Run multiple random seeds; keep best feasible output.