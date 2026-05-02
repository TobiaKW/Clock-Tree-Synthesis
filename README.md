# Clock Tree Synthesis (CTS)

A C++17 implementation of a grid-based Clock Tree Synthesis tool for **CENG4120, CUHK**. It assigns each pin to a tap and routes rectilinear trees from taps to pins while respecting **MAX_LOAD**, **CAPACITY**, and rectangular **blockages**. The objective matches the course evaluator: minimize global skew plus total routed wirelength.

## Features

### Pin-to-tap assignment (`mfmc`)

- **Greedy assignment**: All pin–tap edges sorted by (discounted) Manhattan cost; each pin goes to the best feasible tap under **MAX_LOAD**, with load-based tie-breaking.
- **Discount-based reassignment**: Detects outlier taps via per-tap max assigned distance vs mean/std (**kSigma** gate); lowers that tap’s edge costs and rebuilds with greedy. Bounded iterations.
- **Best-partner 2-swap**: Local refinement; focuses on high-risk taps and top alternative taps per pin; capacity-preserving pairwise swaps when Manhattan gain is positive.

### Routing (`astar`, `grid`)

- **A\*** from each pin to the growing tap tree (goal: connect to any tree node), with **blockage** and **CAPACITY** checks on the grid.
- **Cached closest-tree-distance heuristic** for faster search on large instances.

### Orchestration (`main`)

- **Multi-retry routing**: First retry uses deterministic pin order (near-first) and tap order (heavy-load first); later retries shuffle pin and tap orders.
- **Best solution kept** across retries using cost aligned with **`eval.py`** (unique unit-edge wirelength + skew from BFS delays).
- **Adaptive time budget**: Routing retries stop before a configurable wall-clock limit (with at least one successful routing completed first), plus a hard **max_retry** cap.

## Builds

```bash
g++ -std=c++17 -O2 -Wall -Wextra main.cpp parser.cpp grid.cpp mfmc.cpp astar.cpp -o cts
```

## Usage

```bash
./cts <input_file> <output_file>
```

Example:

```bash
./cts test4.in test4.out
```

## Evaluation & visualization

```bash
python3 eval.py --input test.in --output test.out
python3 eval.py --input testname.in --output testname.out --plot true
```

## Project layout

| Path | Role |
|------|------|
| `main.cpp` | Parse, assign, retry loop, cost, output |
| `mfmc.cpp` / `mfmc.hpp` | Assignment, discount, swap |
| `astar.cpp` / `astar.hpp` | A* routing |
| `grid.cpp` / `grid.hpp` | Edges, capacity, trees |
| `parser.cpp` / `parser.hpp` | Input I/O |
| `types.hpp` | Shared structs |
| `eval.py` | Official-style grading / plots |

## License

See `LICENSE`.
