# Dispatch benchmark: does the choice of infrastructure cost anything?

This benchmark accompanies the TAAPSD 2026 theses *"On the expressiveness limits of
CRTP-based tuple infrastructures under runtime conditions"* (V. Boublik, NaUKMA).

The theses argue that the choice between **tuple + fold** and **variant + visit** is a
question of *expressiveness* (is the set of participants known at compile time?),
not of performance. This benchmark is the evidence for the "not of performance" part,
and an open invitation to challenge it.

## What is measured

Seven ways to run a set of "reports", each report doing `header() + body() + footer()`:

1. GoF: virtual dispatch (Template Method)
2. CRTP
3. Deducing this (C++23)
4. `std::tuple` + `std::apply` + fold expression
5. `std::vector<std::variant>` + `std::visit`
6. Type erasure: `AnyReport` with a hidden `Concept` / `Model<T>` and a C++20 concept (Parent style)
7. `std::function<void()>`: erasing an action rather than an object

Two scenarios:

- **A: composition fixed at compile time.** 3 reports, 1,000,000 repetitions.
  All five approaches can express it.
- **B: composition determined at runtime.** This scenario models `read_user_choice()`
  from the theses: a collection whose composition is unknown at compile time, which is
  the case for variant + visit. Random numbers stand in for the user's choice of
  N reports (default 3000). The seed makes this "choice" identical for everyone who
  runs the test. **Only GoF, variant + visit and the two type-erasure variants can express it**;
  CRTP, deducing this and tuple cannot, whatever the optimization level.

Every variant does exactly the same work (9 writes to a `volatile` sink per
repetition in scenario A). Each measurement is the median of 7 runs after a warm-up.

## Build and run

```
g++ -std=c++23 -O2 -o main main.cpp
./main            # random seed (Windows cmd: main)
./main 3000 42    # N = 3000 reports, fixed seed 42 (reproducible)
```

MSVC: `cl /std:c++latest /O2 /EHsc main.cpp`. Deducing this requires GCC 14+,
Clang 18+ or MSVC 19.32+.

## Results so far (ms, median of 7, N = 3000, seed 42, all pinned to one core)

- **MSVC**: MSVC 19.44 (Visual Studio 2022), Release x64 (`/O2`), Windows 10,
  Lenovo ThinkBook 16 G7, Intel Core Ultra 5 125U
- **GCC**: GCC 16.1 (MSYS2), `-O2`, same machine
- **Clang**: Clang 18.1, `-O2`, Linux, Intel Xeon 2.1 GHz

The control sum `sink = 3489049323` was identical on all three, so the same work was done.

Traversal time, ms:

| Approach | A: MSVC | A: Clang | A: GCC | B: MSVC | B: Clang | B: GCC |
|---|---|---|---|---|---|---|
| GoF (virtual) | 15.0 | 24.0 | 15.5 | 24.5 | 46.8 | 14.3 |
| CRTP | 14.6 | 23.4 | 14.8 | n/a | n/a | n/a |
| Deducing this | 15.2 | 23.0 | 15.7 | n/a | n/a | n/a |
| tuple + fold | 15.0 | 22.9 | 15.6 | n/a | n/a | n/a |
| variant + visit | 13.2 | 23.3 | 15.6 | 14.2 | 24.0 | 15.8 |
| type erasure (AnyReport) | 13.0 | 23.0 | 15.6 | 17.0 | 27.5 | 14.9 |
| std::function | 13.1 | 23.6 | 15.6 | 16.6 | 23.2 | 14.4 |

*A: composition fixed at compile time. B: composition determined at runtime.
n/a = not expressible: the composition is unknown at compile time.*

Building and destroying the scenario-B collection (not included above), µs:

| Approach | Objects stored | Dispatch per report | MSVC | Clang | GCC |
|---|---|---|---|---|---|
| GoF (virtual) | heap, `unique_ptr` | 3 virtual calls | 135.9 | 153.3 | 156.3 |
| variant + visit | inline in the vector | 1 jump on the index | 3.4 | 4.6 | 2.7 |
| type erasure (AnyReport) | heap, `Model<T>` | 1 virtual call | 110.6 | 152.5 | 141.5 |
| std::function | small buffer (in the libraries used) | 1 indirect call | 21.0 | 33.1 | 17.5 |

### Hybrid processors: pin the run to one core

The Core Ultra 5 125U has 2 performance, 8 efficient and 2 low-power efficient cores. Windows
moved the process between core types during the run: without pinning, scenario A spread from
2.9 to 14.4 ms; pinned, from 12.8 to 14.5 ms. Scenario B did not change. On Windows:

```
start /b /wait /affinity 1 benchmark.exe 3000 42
```

On Linux: `taskset -c 0 ./benchmark 3000 42`.

## Interpretation (and where we'd like to be argued with)

- In **scenario A** all seven approaches are close once optimization is on: the compiler
  sees the concrete types and removes the dispatch, including, in practice, the virtual one.
- **Storage versus dispatch.** Allocating every object on the heap costs about 37–52 ns per
  object, against about 1 ns for variant. In the traversal, GoF vs AnyReport (same storage,
  3 calls vs 1) shows the price of the number of calls; AnyReport vs `std::function` (one call
  each, heap vs small buffer) shows the price of memory layout.
- **Scenario B depends on the compiler.** With MSVC and Clang virtual dispatch was the slowest;
  with GCC all four approaches stayed at the floor set by the `volatile` chain (14.3–15.8 ms),
  so the dispatch cost was hidden behind the memory dependency.
- The stable conclusion does not depend on timing: when the composition is determined at
  runtime, the tuple-based infrastructure cannot express the requirement at all.

Open questions we would welcome opinions, measurements or counter-examples on:

1. Is a chain of `volatile` writes a fair "realistic workload", or does it hide the
   dispatch cost that would matter in a tighter loop?
2. How should scenario B be designed so that the branch predictor cannot learn a
   repeated sequence of 3000 reports? Is that even desirable?
3. Results on other compilers and CPUs (please include compiler version, flags and CPU
   model).
4. Other forms of type erasure (`std::any`, `std::move_only_function`, small-buffer
   optimised wrappers): how do they compare in scenario B?

Please share results or arguments in **Discussions** (or open an issue). Pull requests
with improved measurement methodology are welcome.
