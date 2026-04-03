# Clock Tree Synthesis (CTS) — implementation plan

**Overview:** A greenfield C++17 Clock Tree Synthesis tool that reads the course grid/tap/pin/blockage format, assigns each pin to exactly one tap under `MAX_LOAD`, routes rectilinear trees per tap while respecting `CAPACITY` and blockages, and writes tap-by-tap output while minimizing the stated skew-weighted cost plus total wirelength.

## Problem recap (from your spec)

- **Feasibility:** Every pin assigned to exactly one tap; each tap drives at most `MAX_LOAD` pins; each **grid edge** (unit horizontal/vertical segment between adjacent lattice points) carries at most `CAPACITY` routed segments across **all** taps combined; routes are Manhattan and must not pass through blockage **interiors** (boundary routing allowed).
- **Cost to minimize:**

\[
c = (\max_i d_i - \min_j d_j) \times \texttt{numTaps} + \sum_{k} \texttt{TreeLength}(\texttt{TAP}_k)
\]

where delay \(d_i\) is **path length** from tap to pin \(i\) along your routed tree.

```mermaid
flowchart LR
  parse[Parse input] --> assign[Pin-to-tap assignment]
  assign --> route[Per-tap rectilinear trees]
  route --> cap[Capacity and blockage checks]
  cap --> cost[Compute cost]
  cost --> write[Write output]
  cap -->|violations| refine[Refine or rip-up]
  refine --> route
```

## Recommended C++17 layout

Keep modules small and testable:

| Piece | Responsibility |
|--------|----------------|
| `types.hpp` | `Pin`, `Tap`, `Blockage`, `Segment` / `Edge`, `Problem` (globals + collections) |
| `grid.hpp/cpp` | Coordinate bounds (`0..GRID_SIZE`), **edge-id** mapping for capacity (each undirected unit segment → index), blockage point/edge legality |
| `io.hpp/cpp` | Read/write the exact text format from your spec |
| `assign.hpp/cpp` | Initial pin→tap assignment respecting `MAX_LOAD` |
| `route.hpp/cpp` | Build routing per tap; maintain a shared **global** capacity counter |
| `validate.hpp/cpp` | Check connectivity, loads, capacities, blockage; optional tree check |
| `cost.hpp/cpp` | Skew (max/min delay over all pins) and total wirelength |
| `main.cpp` | CLI: `cts <input> <output>` |

This structure can live at the repo root or under `src/`.

## Core representations

1. **Lattice and segments**  
   Pins/taps sit on integer grid points. Output `EDGE x0 y0 x1 y1` are axis-aligned; split any non-unit segment into **unit edges** for capacity and delay.

2. **Blockages**  
   Precompute, for each unit grid edge, whether it **crosses** a rectangle’s interior (forbidden) vs only touches the boundary (allowed per spec). Reuse this for routing and validation.

3. **Global capacity**  
   Maintain `vector<int> use(countEdges)` (or similar). Every time a unit segment is used by any tap’s tree, increment; never exceed `CAPACITY`.

## Algorithm pipeline (practical two-phase approach)

### Phase 1 — Pin assignment (under MAX_LOAD)

Start with a **feasible** assignment, then optionally improve:

- **Feasibility check:** `numPins <= numTaps * MAX_LOAD` (necessary).
- **Initial assignment:** Greedy by Manhattan distance to nearest tap with remaining capacity, or **min-cost max-flow** on a bipartite graph (pins → taps) with arc capacities `MAX_LOAD` and costs = distance — good for moderate sizes and balances load vs distance.
- **Skew awareness:** The cost couples **global** skew with `numTaps`. After routing you will know true delays; consider **iterative refinement**: swap pins between taps or re-balance loads if skew dominates (local search or a few ILP variables only if benchmarks stay small).

### Phase 2 — Routing per tap (rectilinear tree + capacity)

For each tap and its pin set:

1. **Topology:** Build a rectilinear tree over `{tap} ∪ assigned pins`. Practical options:
   - **Rectilinear MST** (e.g. Prim/Kruskal on Manhattan metric) on the terminal set, then **L-shaped** or shortest-path embedding of MST edges; or
   - **Sequential Steiner heuristic:** connect terminals one-by-one with shortest feasible paths while respecting blockage and remaining capacity.

2. **Path embedding:** For each tree edge between two points, run **A\*** (or BFS) on the **grid graph** where each move costs 1 and illegal edges (blockage interior / over capacity) are blocked. This yields unit segments for delay and capacity updates.

3. **Conflicts:** If a tap’s routing fails or violates capacity, **backtrack**: try alternate MST order, reroute a subtree, or return to Phase 1 with a different assignment.

**Order of taps:** Route taps in an order that reduces capacity clashes (e.g. larger trees first, or random restarts for hard cases).

### Phase 3 — Output and verification

- Emit only taps that drive at least one pin (per typical interpretation); if the spec requires **every** tap listed even with zero pins, match the reference checker — add a one-line note in code once you have official samples.
- **Validate** before writing: all pins reachable from their tap, rectilinear edges, blockage rules, per-tap load, global capacity, tree structure (no unintended cycles if you want strict “tree”).

### Phase 4 — Cost computation

- For each pin, \(d_i\) = sum of unit segment lengths from tap to pin along the routed tree.
- \(\text{skew} = \max_i d_i - \min_j d_j\) over **all** pins.
- Wirelength = sum of Manhattan lengths of all output `EDGE` segments (each `EDGE` line once; avoid double-counting if you store unit edges internally).

## Build and quality

- **Build:** `g++ -std=c++17 -O2 -Wall -Wextra *.cpp -o cts` (or CMake if you prefer).
- **Tests:** Hand-written tiny grids (1–2 blockages, 2–3 taps) with known feasible routes; golden checks on parser round-trip and cost formula.
- **Debugging:** Optional dump of edge-usage heatmap for a small case.

## Risks and mitigations

| Risk | Mitigation |
|------|------------|
| Capacity infeasible even with valid assignment | Min-cost flow assignment + multi-start; relax with iterative rerouting |
| Skew vs wirelength tradeoff | Tune assignment weights; local pin moves between taps |
| Large benchmarks | Profile A\*; consider bucketed BFS for uniform costs |

## Suggested milestones

1. Parser + writer + blockage/edge legality + manual golden I/O.
2. Assignment only + trivial star routing (tap to each pin) to validate capacity/blockage plumbing.
3. Full MST + A\* per edge + global capacity.
4. Validator + cost + polish / refinement loop.

## Implementation checklist

- [ ] Add C++17 project skeleton: types, grid edge indexing, blockage checks
- [ ] Implement input parser and output writer per course I/O spec
- [ ] Pin-to-tap assignment (greedy + optional min-cost flow) with `MAX_LOAD`
- [ ] Per-tap rectilinear MST + A\* path embedding with global `CAPACITY`
- [ ] Validator (connectivity, loads, blockages, capacity) and cost formula
- [ ] Tiny synthetic tests; run against official benchmarks when available
