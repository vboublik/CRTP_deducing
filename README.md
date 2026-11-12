# CRTP Design Patterns in Modern C++

Source code accompanying the works by V. Boublik (National University of Kyiv-Mohyla Academy):

- *Design Patterns: From CRTP to Deducing This in Modern C++*. Preprint, Zenodo, 2026.
  DOI: [10.5281/zenodo.21408968](https://doi.org/10.5281/zenodo.21408968)
- *Design Patterns: On the Expressiveness Limits of CRTP-Based Tuple Infrastructures under
  Runtime Conditions*. Preprint, Zenodo, 2026 (extended version of the TAAPSD 2026 theses).
  DOI: [10.5281/zenodo.24118157](https://doi.org/10.5281/zenodo.24118157)
- *On the Expressiveness Limits of CRTP-Based Tuple Infrastructures under Runtime Conditions*.
  Theses, TAAPSD 2026.

## Files

| File | What it is |
|---|---|
| `CRTP.cpp` | GoF patterns implemented via virtual functions, CRTP and deducing this (C++23) |
| `static_reports.cpp` | tuple + fold vs variant + visit: the example from the TAAPSD theses and talk, with a new operation (`Save`) and a new type (`XmlReport`, commented out) |
| `benchmark.cpp` | Dispatch benchmark: seven approaches (incl. type erasure and `std::function`), compile-time and runtime composition |
| `ep_demo/` | Expression Problem demo with separate compilation: which extensions need recompiling old code (virtual, variant, type erasure, tuple with external overloads). Run `build.bat` (Windows) or `bash build.sh` (Linux/Mac/Codespaces) |
| [`BENCHMARK.md`](BENCHMARK.md) | Benchmark description, results and open questions |

Requires C++23 (GCC 14+, Clang 18+ or MSVC 19.32+).

## Discussion

Results on other compilers and CPUs, criticism of the methodology and counter-examples
are very welcome: please use **Discussions** or open an **Issue**.

## License

MIT
