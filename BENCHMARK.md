# Dispatch benchmark: does the choice of infrastructure cost anything?

This benchmark accompanies the TAAPSD 2026 theses *"On the expressiveness limits of
CRTP-based tuple infrastructures under runtime conditions"* (V. Boublik, NaUKMA).

The theses argue that the choice between **tuple + fold** and **variant + visit** is a
question of *expressiveness* (is the set of participants known at compile time?),
not of performance. This benchmark is the evidence for the "not of performance" part,
and an open invitation to challenge it.

## What is measured

Five ways to run a set of "reports", each report doing `header() + body() + footer()`:

1. GoF: virtual dispatch (Template Method)
2. CRTP
3. Deducing this (C++23)
4. `std::tuple` + `std::apply` + fold expression
5. `std::vector<std::variant>` + `std::visit`

Two scenarios:

- **A: composition fixed at compile time.** 3 reports, 1,000,000 repetitions.
  All five approaches can express it.
- **B: composition determined at runtime.** This scenario models `read_user_choice()`
  from the theses: a collection whose composition is unknown at compile time, which is
  the case for variant + visit. Random numbers stand in for the user's choice of
  N reports (default 3000). The seed makes this "choice" identical for everyone who
  runs the test. **Only GoF and variant + visit can express it**; CRTP, deducing this
  and tuple cannot, whatever the optimization level.

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

## Results so far (ms, median of 7)

Machines and builds:

- **GCC**: GCC 16.1 (MSYS2), `-O2`, x64, Windows 10, Intel Core Ultra 5 125U
- **MSVC**: MSVC 19.44 (Visual Studio 2022), Release x64 (`/O2`), same machine
- **Clang**: Clang 18.1, `-O2`, x64, Linux, Intel Xeon 2.1 GHz

Scenario A: composition fixed at compile time

| Approach | GCC | MSVC | Clang |
|---|---|---|---|
| GoF (virtual) | 15.72 | 14.51 | 24.88 |
| CRTP | 15.18 | 15.19 | 22.51 |
| Deducing this | 15.51 | 13.19 | 22.73 |
| tuple + fold | 15.47 | 14.16 | 23.42 |
| variant + visit | 15.52 | 11.00 | 23.42 |

Scenario B: composition determined at runtime

| Approach | GCC | MSVC | Clang |
|---|---|---|---|
| GoF (virtual) | 14.01 | 24.98 | 36.71 |
| CRTP / deducing this / tuple | n/a | n/a | n/a |
| variant + visit | 15.58 | 14.63 | 24.09 |

*n/a = not expressible: the composition is unknown at compile time.*

## Interpretation (and where we'd like to be argued with)

- In **scenario A** all five approaches are close once optimization is on: the
  compiler sees the concrete types and removes the dispatch, including, in practice,
  the virtual one. GCC and Clang are within measurement noise; MSVC shows a wider
  spread (11–15 ms) that we have not yet explained.
- In **scenario B** the result depends on the compiler: with MSVC and Clang,
  variant + visit was 1.5–1.7× faster than virtual dispatch; with GCC, virtual
  dispatch was ~10% faster. So variant + visit was never noticeably slower, but we do
  **not** claim it is faster in general.
- The stable conclusion is the one that does not depend on timing: when the
  composition is determined at runtime, the tuple-based infrastructure cannot express
  the requirement at all.

Open questions we would welcome opinions, measurements or counter-examples on:

1. Is a chain of `volatile` writes a fair "realistic workload", or does it hide the
   dispatch cost that would matter in a tighter loop?
2. How should scenario B be designed so that the branch predictor cannot learn a
   repeated sequence of 3000 reports? Is that even desirable?
3. Results on other compilers and CPUs (please include compiler version, flags and CPU
   model).
4. Where would type erasure (`std::function`, `std::any`, Sean Parent–style concepts)
   sit in this table?

Please share results or arguments in **Discussions** (or open an issue). Pull requests
with improved measurement methodology are welcome.
