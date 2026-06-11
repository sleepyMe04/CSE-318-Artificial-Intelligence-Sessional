# Online Assignment – Decision Tree Evaluation (CSE318)

## Problem Statement

An in-lab online contest extending the Decision Tree assignment. The task involves running **systematic experiments** to compare different attribute selection criteria across multiple random train/test splits, and reporting statistical summaries of classifier performance.

---

## What It Does

The program runs **20 independent experiments** for a given criterion and max depth setting:

1. Shuffles the full dataset with a fresh random seed each run
2. Splits 80% for training, 20% for testing
3. Builds the decision tree using the selected criterion
4. Evaluates accuracy on the test set
5. Records tree size (node count) and depth

At the end, it reports:
- Average, min, and max accuracy across all 20 runs
- Average number of nodes in the tree
- Average tree depth

---

## Sample Output

```
Running 20 experiments with information_gain and maxDepth 10...
--------------------------------------------------
Run  1/20 - Accuracy: 84.52%, Nodes: 312, Depth:  9
Run  2/20 - Accuracy: 85.11%, Nodes: 298, Depth: 10
...
==================================================
FINAL RESULTS - information_gain with maxDepth 10
==================================================
Average Accuracy: 84.87%
Min Accuracy: 83.20%
Max Accuracy: 86.40%
Average Nodes: 305.4
Average Depth: 9.7
```

---

## Supported Datasets

| Dataset | File | Task |
|---------|------|------|
| Adult Income | `adult.data` | Binary income classification |
| Iris | `iris.csv` | 3-class flower classification |

---

## How to Compile and Run

```bash
g++ -O2 -std=c++17 -o online 2105114\ \(1\).cpp
./online
```

Select dataset (1 = Adult, 2 = Iris), criterion (e.g. `information_gain`, `gain_ratio`), and max depth when prompted.

---

## Notes

- Each run uses `chrono::high_resolution_clock` seeded with the run index to ensure different shuffles per run while remaining reproducible within a session.
- Tree memory is freed after each run via `freeTree()`.
- Unique attribute values are logged at startup (for Adult dataset) to help diagnose discretization behavior.