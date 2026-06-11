# Assignment 4 – Decision Tree Classifier (ID3)

## Problem Statement

Implement a **Decision Tree** classifier from scratch, train it on real-world datasets, and evaluate performance using different attribute selection criteria. The implementation must handle:
- Mixed attribute types (categorical and numeric)
- Missing values
- Overfitting via depth limiting

Tested on two datasets: **Adult Income** (census data) and **Iris** (flower classification).

---

## Algorithm

The tree is built recursively using the **ID3** framework:

```
BuildTree(examples, attributes):
    if all examples have the same label → return leaf with that label
    if no attributes remain or max depth reached → return leaf with majority class
    best_attr = chooseBestAttribute(examples, attributes, criterion)
    for each value v of best_attr:
        subtree = BuildTree(examples where best_attr = v, attributes - {best_attr})
        add branch best_attr = v → subtree
```

---

## Attribute Selection Criteria

Four criteria are implemented for choosing the best split attribute:

| Criterion | Formula | Notes |
|-----------|---------|-------|
| **Information Gain (IG)** | `IG = H(S) - Σ(|Sv|/|S|) × H(Sv)` | Biased toward high-cardinality attributes |
| **Gain Ratio (IGR)** | `IGR = IG / IntrinsicValue` | Normalizes IG by split information |
| **Normalized Weighted IG (NWIG)** | Custom penalty term on number of splits | Balances breadth and gain |
| **Gini Impurity** | `Gini = 1 - Σ pᵢ²` | Alternative impurity measure |

---

## Key Features

### Numeric Attribute Discretization
Numeric attributes are converted to categorical buckets before tree construction. Domain-specific bins are defined for the Adult dataset:

| Attribute | Bins |
|-----------|------|
| `age` | `<25`, `25-34`, `35-44`, `45-54`, `55-64`, `65+` |
| `hours-per-week` | `very_part_time`, `part_time`, `full_time`, `over_time`, `extreme` |
| `capital-gain` | `none`, `very_low`, `low`, `medium`, `high` |
| `fnlwgt` | `low`, `medium_low`, `medium`, `medium_high`, `high` |

### Missing Value Handling
Missing values (marked as `?`, empty, `Unknown`, or `NA`) are replaced with the **mode** (most frequent value) of that attribute across the training split.

### Dataset Configuration
A `DatasetConfig` struct cleanly separates dataset-specific settings (delimiter, header presence, attribute names, label column index) from the tree-building logic.

---

## Datasets

### Adult Income (`adult.data`)
- **Source:** UCI Machine Learning Repository
- **Size:** ~48,842 examples, 14 attributes
- **Task:** Predict whether income > $50K or ≤ $50K
- **Delimiter:** `, ` (comma + space)

### Iris (`Iris.csv`)
- **Source:** UCI Machine Learning Repository
- **Size:** 150 examples, 5 attributes (4 numeric)
- **Task:** Classify into Setosa, Versicolor, Virginica
- **Delimiter:** `,`

---

## How to Compile and Run

```bash
g++ -O2 -std=c++17 -o dtree 2105114.cpp
./dtree
```

The program reads the dataset, splits into training/test sets (80/20), builds the tree with a configurable criterion and max depth, and reports accuracy.

---

## Implementation Notes

- Tree nodes are allocated on the heap (`TreeNode*`). Leaf nodes carry a `label`; internal nodes carry an `attribute` name and a `children` map from attribute value → child node.
- `splitData()` performs mode-imputation for missing values in a two-pass approach: first collecting value counts to find the mode, then splitting.
- Depth limiting is the primary regularization mechanism to prevent overfitting on high-cardinality datasets like Adult.