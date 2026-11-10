# Fixed-Slot Render Lookup Cache & Benchmark (C++17)

Header-only bounded O(1) validation cache and benchmark harness demonstrating how to eliminate single-threaded frame-loop saturation caused by repeated per-frame memory/asset validation checks.

## Engineering Context

In real-time client/render pipelines, invoking OS memory checks (such as `VirtualQuery`) or linear descriptor scans inside a hot render path saturates the main thread without necessarily leaking memory. Replacing per-frame probes with a fixed-capacity, epoch-invalidatable lookup table restores frame pacing while preserving pointer/bounds safety.

## Features

- **Zero heap allocations on lookup:** Pre-allocated `std::array` slot table indexed by masked power-of-two slot or bounded ID.
- **Explicit bounds & epoch invalidation:** Rejects out-of-range entity/model IDs (`>= MaxEntries`) and allows O(1) cache invalidation by bumping the epoch counter without scanning or reallocating memory.
- **Strict compiler compliance:** Built and tested under `-std=c++17 -O2 -Wall -Wextra -Werror`.

## Build & Run

```bash
g++ -std=c++17 -O2 -Wall -Wextra -Werror -Iinclude tests/cache_tests.cpp -o cache_tests
./cache_tests

g++ -std=c++17 -O2 -Wall -Wextra -Werror -Iinclude src/benchmark.cpp -o benchmark
./benchmark
```
