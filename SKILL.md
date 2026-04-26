---
name: perf-ninja-runner
description: Build, validate, benchmark, and document every lab in the perf-ninja repository on a Linux host, producing a per-lab `RESULTS.md` with intuition-first explanations and a cross-lab `PATTERNS.md` summarizing the optimization techniques the labs teach.
type: process
---

# Skill: Run all perf-ninja labs and produce intuition-first results

## What this skill does

Performance Ninja (https://github.com/dendibakh/perf-ninja) is a hands-on
course where each lab is a small C++ program with a *deliberate* performance
problem. Every lab ships with:

* a baseline `solution.cpp` (slow on purpose),
* a reference `validate.cpp` (correctness check),
* a Google Benchmark harness in `bench.cpp`.

The goal of this skill is to **run every lab in its baseline form**, capture
what the baseline does and how slow it is, and write the result in a way
that a reader who has *not* read the textbook can still build mental models
about what's going wrong and why — i.e. develop *intuition* before reaching
for tooling.

The output is two things:

1. A `RESULTS.md` next to every lab (e.g.
   `labs/core_bound/vectorization_1/RESULTS.md`) describing that one lab in
   plain English: what the workload is, what the baseline number is, and the
   *symptom-to-cause* pattern that explains the slowdown.
2. A repository-level `PATTERNS.md` that summarizes the recurring patterns
   across all labs, indexed so a reader can jump from "I see this symptom in
   my profile" → "study these labs."

## Reuse-by-context (excerpt from the project README)

> Performance Ninja is an online course where you can learn to find and fix
> low-level performance issues, for example CPU cache misses and branch
> mispredictions. It's all about practice — the course is delivered as a set
> of lab assignments and YouTube videos. Each lab focuses on a specific
> performance problem (Core Bound, Memory Bound, Bad Speculation, Frontend
> Bound, Misc) and has a baseline workload with a known bottleneck for the
> learner to find and fix. Submissions are benchmarked by a CI runner on
> reference hardware (Alder Lake, Coffee Lake, Zen3, M1).

This skill *does not solve* the labs. It runs the unmodified baselines and
explains them, so a learner can compare their own optimized numbers against
the same starting point.

## Prerequisites

The host must have:

* `cmake ≥ 3.13`, `git`, `python3`
* A C++17 compiler — clang-17 is what the upstream CI uses; gcc-13+ works
  too. The QuickstartLinux.md instructions install clang-17 + alternatives.
* Google Benchmark, built into `tools/benchmark/build/` (the helper script
  is `tools/make_benchmark_library.sh`).
* `linux-tools-*` if you want to set the CPU governor with `cpupower`. This
  is optional — without it, expect noisier numbers.

## Procedure

### 1. One-time setup

```bash
sudo apt-get install -y build-essential cmake ninja-build clang-17 git python3 \
                        linux-tools-common linux-tools-generic
sudo update-alternatives --install /usr/bin/cc  cc  /usr/bin/clang-17   30
sudo update-alternatives --install /usr/bin/c++ c++ /usr/bin/clang++-17 30
sudo cpupower frequency-set --governor performance || true
```

Then build the benchmark dependency once:

```bash
cd ~/dev/perf-ninja/tools && bash make_benchmark_library.sh
```

### 2. Per-lab run

Each lab uses an identical CMake skeleton from `tools/labs.cmake`, so the
build/run steps are the same for every lab:

```bash
cd labs/<category>/<lab>
cmake -E make_directory build && cd build
cmake -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_C_COMPILER=clang-17 \
      -DCMAKE_CXX_COMPILER=clang++-17 ..
cmake --build . --config Release --parallel "$(nproc)"
cmake --build . --target validateLab     # must print "Validation Successful"
./lab --benchmark_min_time=1s --benchmark_repetitions=3 \
      --benchmark_report_aggregates_only=true
```

For labs with extra build steps (`misc/lto`, `misc/pgo`) — follow each
lab's local `README.md`; the runner script in step 3 detects and handles
them.

### 3. Batch runner

A single bash loop walks every `CMakeLists.txt` under `labs/`:

```bash
for lab in $(find labs -name CMakeLists.txt -not -path '*/build/*' | sort); do
  dir=$(dirname "$lab")
  pushd "$dir" >/dev/null
  rm -rf build && mkdir build && cd build
  cmake -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_C_COMPILER=clang-17 \
        -DCMAKE_CXX_COMPILER=clang++-17 .. >cmake.log 2>&1
  cmake --build . --config Release --parallel "$(nproc)" >>cmake.log 2>&1
  cmake --build . --target validateLab >validate.log 2>&1
  ./lab --benchmark_min_time=1s --benchmark_repetitions=3 \
        --benchmark_format=json --benchmark_out=bench.json \
        --benchmark_report_aggregates_only=true >bench.txt 2>&1 || true
  popd >/dev/null
done
```

The runner stores three artifacts per lab inside `build/`:
`cmake.log`, `validate.log`, `bench.txt`, `bench.json`. Failures (build
error, validation failure, missing perf counter, missing AVX2 etc.) are
*not* treated as fatal — they are recorded honestly in the lab's
`RESULTS.md`.

### 4. RESULTS.md template (intuition-first)

For every lab, write a `RESULTS.md` next to the lab's `README.md` that
follows this five-section structure. The key shift from the upstream
README is **lead with the picture, not the algorithm name**.

```markdown
# <Lab name> — Results

## In one paragraph
What workload runs, what the baseline does, and what the *bottleneck class*
is in plain English (no acronyms in the first sentence).

## The mental picture
A 2–4 sentence analogy or diagram that explains the bottleneck without code.
Examples:
* Branch mispredict: "the CPU is guessing which way an `if` will go and
  paying the price every time it guesses wrong."
* False sharing: "two threads are fighting over the same 64-byte cache line
  even though they're touching different variables inside it."
* Recurrence: "each iteration has to wait for the previous one to finish a
  multiplication before it can start its own."

## Numbers
- Validation: ✅ / ❌
- Baseline mean wall time / op
- Run-to-run stability (CV %)
- Caveats from this host (no AVX2, QEMU CPU, etc.)

## What the optimization will look like
One paragraph hint at *what kind of fix* the lab is teaching — without
spoiling the solution. E.g. "the fix is a data-layout change, not a
better algorithm."

## Pattern this belongs to
Link into `PATTERNS.md` so the reader can see the family the lab belongs
to.
```

### 5. Cross-lab `PATTERNS.md`

After every per-lab file is written, aggregate the patterns observed into
a single repo-level guide. Group by Top-Down category (Core Bound, Memory
Bound, Bad Speculation, Frontend, Misc). For each pattern, list:

* the *symptom* in a profile (e.g. "high branch-miss rate", "high LLC-miss
  rate", "high backend stalls with low IPC");
* the *cause* one-liner;
* the *labs* that exemplify it.

This produces a "if your profile looks like X, study labs Y, Z" lookup.

### 6. Submission

```bash
git checkout -b results/<descriptive-name>
git add labs/**/RESULTS.md PATTERNS.md SKILL.md
git commit -m "docs: baseline results + intuition guide for all labs"
git push -u origin results/<descriptive-name>
```

If pushing to a fork, set the remote with SSH (preferred — see the
"Safely configuring the VM to push to GitHub" instructions) rather than
embedding a PAT in the URL.

## Safety / honesty rules

* **Do not modify `solution.cpp`, `validate.cpp`, or `bench.cpp`.** This
  skill captures *baseline* results; touching the lab source defeats the
  purpose.
* **Record failures, don't hide them.** A lab that doesn't build or
  validates incorrectly on this host gets a `RESULTS.md` that says exactly
  that, with the error excerpt. Future readers need an honest map of what
  worked here.
* **Note the hardware ceiling.** If the host lacks AVX2/AVX-512, say so.
  Speedups for vectorization labs are smaller on a SSE-only host, and a
  reader on different hardware should know.
* **Keep `RESULTS.md` short.** One screen of intuition beats three pages
  of numbers nobody will read. The `bench.json` artifact is there for
  anyone who wants the raw data.
