# Assignment 2 – Max Cut Problem using GRASP

## Problem Statement

Given a weighted undirected graph G = (V, E), partition the vertices into two sets S and S̄ such that the **total weight of edges crossing the partition is maximized**. This is the **Maximum Cut (Max Cut)** problem, which is NP-hard.

The assignment benchmarks four algorithmic strategies against the [G-Set benchmark suite](https://web.stanford.edu/~yyye/yyye/Gset/) of 54 graphs.

---

## Algorithms Implemented

### 1. Randomized Algorithm
Each vertex is independently assigned to set S or S̄ with equal probability (coin flip). Run over 100 iterations and averaged. Serves as a probabilistic baseline — expected to achieve at least **½ of the optimal** cut.

### 2. Simple Greedy
Initializes with the two endpoints of the maximum-weight edge on opposite sides. Each remaining vertex is placed in the set that maximizes the current cut value (based on the weight of its already-assigned neighbors).

### 3. Semi-Greedy (GRASP Construction Phase)
A randomized greedy that builds the solution incrementally:
- At each step, computes a gain score for assigning each unassigned vertex to either partition.
- Builds a **Restricted Candidate List (RCL)** using the parameter `α` — only vertices whose score is within `α × (max − min)` of the best are eligible.
- Randomly selects from the RCL, then assigns the vertex to the better-scoring partition.

### 4. Local Search
Starting from any partition, iteratively flips a vertex to the opposite set if the flip improves the cut value. Repeats until no improving flip exists (local optimum).

### 5. GRASP (Greedy Randomized Adaptive Search Procedure)
Combines Semi-Greedy construction with Local Search refinement in multiple iterations:
```
for i in 1..GRASP_ITERATIONS:
    partition = semiGreedyMaxCut(α)
    cut = localSearch(partition)
    if cut > bestCut:
        bestCut = cut
```
Run with `α = 0.5` and 50 iterations.

---

## Results

Results are saved to `maxcut_results.csv` with the following columns:

| Column | Description |
|--------|-------------|
| Name | Graph file name (g1–g54) |
| \|V\| | Number of vertices |
| \|E\| | Number of edges |
| Randomized | Average cut over 100 runs |
| Greedy | Cut from simple greedy |
| Semi-Greedy | Cut from one semi-greedy run (α=0.5) |
| Local Search | Average cut over 10 local search runs |
| GRASP | Best cut over 50 GRASP iterations |
| Known Best | Known optimal or best-known solution |

### Visualizations

Use `2105114_plot.py` to generate bar charts comparing all algorithms across all 54 graphs (grouped in sets of 10):

```bash
python 2105114_plot.py
```

---

## How to Compile and Run

> **Note:** The current source uses Windows API (`<windows.h>`) for directory traversal. On Linux/macOS, replace the `getGraphFiles()` function with a `std::filesystem::directory_iterator` equivalent.

```bash
# Windows (MinGW/MSVC)
g++ -O2 -o maxcut maxcut_final.cpp
./maxcut
```

Place the graph files in a folder named `set1/` in the same directory. Output is written to `maxcut_results.csv`.

---

## Graph File Format (`.rud`)

```
<num_vertices> <num_edges>
<src> <dest> <weight>
<src> <dest> <weight>
...
```

---

## Known Best Solutions (Selected)

| Graph | \|V\| | \|E\| | Known Best |
|-------|--------|--------|------------|
| G1 | 800 | 19176 | 12078 |
| G22 | 2000 | 19990 | 14123 |
| G48 | 3000 | 6000 | 6000 |

Full benchmark data from: Ye, Y. (1999). *GSET: A graph dataset for Max-Cut benchmarking.*

---

## Parameters

| Parameter | Value Used |
|-----------|------------|
| α (GRASP greediness) | 0.5 |
| Randomized iterations | 100 |
| Local search runs | 10 |
| GRASP iterations | 50 |
