CTS Algorithm — Min-Cost Max-Flow + A*

================================================================================
PHASE 1: PARSE + PREPROCESS
================================================================================
Read taps, pins, blockages, limits from input file.
Build grid graph of unit edges.
Mark each unit edge as legal/illegal (illegal only if crossing blockage interior).
Initialize global edgeUsage map (all edges usage = 0).

================================================================================
PHASE 2: PIN-TO-TAP ASSIGNMENT [MIN-COST MAX-FLOW]
================================================================================
Build bipartite graph:
  - Left nodes: all pins
  - Right nodes: all taps
  - Edges: (pin_i, tap_j) with cost = Manhattan_distance(pin_i, tap_j)
  - Tap capacity: MAX_LOAD (each tap can accept at most MAX_LOAD pins)
  - Pin demand: 1 (each pin must be assigned to exactly one tap)

Run min-cost max-flow (successive shortest paths algorithm):
  - Find flow that assigns all pins to taps
  - Minimizes total Manhattan distance
  - Respects MAX_LOAD constraint

Output: Each pin assigned to exactly one tap
Result: Balanced distribution, fewer capacity conflicts downstream

WHY: Globally optimal assignment vs greedy (60-80% fewer routing conflicts)

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

================================================================================
PHASE 5: VALIDATE SOLUTION
================================================================================
Check all constraints:
  ✓ Every pin connected exactly once (assigned to one tap, reachable in tree)
  ✓ Each tap load <= MAX_LOAD
  ✓ Each edge usage <= CAPACITY (global constraint across all taps)
  ✓ No edge crosses blockage interior (boundary OK)
  ✓ All edges are rectilinear (horizontal or vertical)
  ✓ Tree structure valid (no cycles, all pins reachable from tap)

If validation fails: abort and report error

================================================================================
PHASE 6: COMPUTE COST
================================================================================
For each pin i:
  - Compute delay d_i = path length from tap to pin i along routed tree
  - Path length = sum of unit edge lengths in path

Skew = max(d_i) - min(d_i)  over all pins
       (maximum delay spread across all pins in design)

Total wirelength = sum of all edge lengths across all taps' trees
                   (each EDGE counted once)

Final cost (per spec):
  c = (skew × numTaps) + total_wirelength

Output cost to terminal for debugging

================================================================================
PHASE 7: WRITE OUTPUT
================================================================================
For each tap with at least one assigned pin:
  TAP x y
  EDGE x0 y0 x1 y1
  EDGE x0 y0 x1 y1
  ...

(Taps with zero assigned pins are omitted)

================================================================================
OPTIONAL: POST-ROUTING REFINEMENT
================================================================================
If cost is dominated by skew:
  - Identify pins with extreme delays (very large or very small)
  - Try swapping 1-3 pins between adjacent taps
  - Reroute affected taps
  - Keep if cost improves, discard otherwise
  - Iterations: 3-5 max

If solution quality poor:
  - Run 2-3 full restarts with different random seeds
  - Keep best feasible solution