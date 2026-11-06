# CRTP Design Patterns in Modern C++

Source code accompanying two works by V. Boublik (National University of Kyiv-Mohyla Academy):

- *Design Patterns: From CRTP to Deducing This in Modern C++*. Preprint, Zenodo, 2026.
  DOI: [10.5281/zenodo.21408968](https://doi.org/10.5281/zenodo.21408968)
- *On the Expressiveness Limits of CRTP-Based Tuple Infrastructures under Runtime Conditions*.
  Theses, TAAPSD 2026.

## Files

| File | What it is |
|---|---|
| `CRTP.cpp` | GoF patterns implemented via virtual functions, CRTP and deducing this (C++23) |
| `static_reports.cpp` | tuple + fold vs variant + visit: the example from the TAAPSD theses |
| `benchmark.cpp` | Dispatch benchmark: five approaches, compile-time and runtime composition |
| [`BENCHMARK.md`](BENCHMARK.md) | Benchmark description, results and open questions |

Requires C++23 (GCC 14+, Clang 18+ or MSVC 19.32+).

## Discussion

Results on other compilers and CPUs, criticism of the methodology and counter-examples
are very welcome: please use **Discussions** or open an **Issue**.

## License

MIT
