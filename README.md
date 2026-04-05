# Clock Tree Synthesis (CTS)

A C++17 implementation of a Clock Tree Synthesis tool that routes clock signals from source taps to destination pins while minimizing skew and wirelength under capacity and blockage constraints. This program serves as the course project in CENG4120, CUHK.

## Planned Features

- **Pin-to-Tap Assignment**: Min-cost max-flow optimization
- **Rectilinear Routing**: A* pathfinding with capacity tracking
- **Blockage Support**: Interior crossing detection with boundary routing
- **Global Capacity Tracking**: Enforces edge usage limits across all taps

## Build

```bash
g++ -std=c++17 -O2 -Wall *.cpp -o cts
```

## Usage

```bash
./cts <input_file> <output_file>
```

## ⚠️ Disclaimer

**This project is a Work In Progress (WIP).** 

- Features are incomplete and subject to change
- Not all functionality has been thoroughly tested
- Performance optimizations are pending
- API and file formats may change without notice
- Use at your own risk.
