# Clustering Processor

Emilio Cazares Borbon  
Georgetown University - COSC 3590 Data Mining  
Spring 2026

## What this project contains

This repository implements a k-means clustering algorithm with comprehensive data preprocessing and experimental analysis capabilities. The project processes ARFF format datasets and supports multiple clustering configurations with detailed metrics collection.

- **C++ runner** (`processor_main`): Custom k-means implementation with parallelization
- **Test suite** (`processor_test`): Unit and integration tests for core components

Both executables write run artifacts under `results/` organized by experiment timestamp for easy comparison and analysis.

## Project layout

- `src/` - C++ implementation and test sources
- `include/` - C++ headers (main.h, clustering_processor.h)
- `data/` - input datasets (`adult-big.arff`, `adult-small.arff`)
- `results/` - generated run outputs with timestamped directories
- `external/arff/` - ARFF parsing dependency
- `design.txt` - detailed design document

## Prerequisites

### C++ pipeline
- CMake 3.22+
- C++17 compiler (clang++/g++)

## Build the C++ executables

From project root:

```bash
cmake -B build
cmake --build build
```

This builds:

- `build/processor_main`
- `build/processor_test`

## Usage

### Quick run (uses defaults)

```bash
./build/processor_main
```

Default values:

- dataset: `data/adult-big.arff`
- class attribute: `class`
- k values: `2 3 4 5 6 7 8 9 10`
- termination strategy: `SSE`
- max threads: `10`

### Full CLI form

```bash
./build/processor_main <dataset_path> <class_attribute> <k_values...> [--threads=N] [--strategy=STRATEGY]
```

Example:

```bash
./build/processor_main data/adult-small.arff class 3 5 7 --threads=4 --strategy=SSE
```

### Configuration options

- `dataset_path`: Path to ARFF input file
- `class_attribute`: Name of the class/target attribute
- `k_values`: Space-separated list of cluster counts to evaluate
- `--threads=N`: Maximum number of threads for parallelization (default: 10)
- `--strategy=STRATEGY`: Termination strategy - `SSE` or `ONE` (default: SSE)

## Output

Each run writes results under `results/` in timestamped folders, for example:
- `results/run_YYYYMMDD_HHMMSS_run_001/`

Inside each run folder:
- `summary_k_XXX.txt` - Textual summary for k=XXX
- `clusters_k_XXX.csv` - Detailed cluster assignments and centroids
- `composition_k_XXX.txt` - Class distribution within clusters
- `experiment_metrics.csv` - Aggregated metrics across all k values

### Metrics included

- **Runtime**: Total execution time in microseconds
- **SSE**: Sum of squared errors (intra-cluster distances)
- **Inter-cluster distance**: Average distance between cluster centroids
- **Intra-cluster distance**: Average distance of instances to their centroid
- **Cluster composition**: Class label distribution per cluster
- **Convergence**: Number of iterations to reach termination criteria

## Optional: Run Tests

```bash
./build/processor_test
```

## Notes

- If you rebuild often, you can use:

```bash
cmake -B build && cmake --build build
```

- All numeric attributes are z-score normalized (mean=0, stddev=1) before clustering
- Missing values are replaced using mean/mode (MEAN_MODE strategy) during preprocessing
- Class attribute is preserved but not used in distance calculations
- Seed is fixed (42) for reproducible clustering across runs
- Results maintain complete iteration history for convergence analysis
