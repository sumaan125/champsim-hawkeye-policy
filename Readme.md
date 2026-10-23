# Hawkeye Cache Replacement Policy in ChampSim

This repository contains a modular implementation of the **Hawkeye Cache Replacement Policy** integrated into the [ChampSim](https://github.com/ChampSim/ChampSim) trace-driven simulator. 

This implementation is based on the ISCA 2016 paper: *"Back to the Future: Leveraging Belady's Algorithm for Improved Cache Replacement"* by A. Jain and C. Lin. The policy learns from Belady's optimal replacement algorithm (OPT) applied to past access streams. By training a predictor on what *would* have been optimal in the past, it predicts whether an incoming cache line will be "cache-friendly" or "cache-averse."

## Project Architecture & Implementation Details

The implementation is broken down into standalone algorithmic components, independent of ChampSim's internal types, and a thin interface adapter. All custom logic resides inside the `ChampSim/` directory.

### 1. OPTgen: Reconstructing Past Optimal Behavior
**Files:** `ChampSim/replacement/hawkeye/optgen.h`, `ChampSim/replacement/hawkeye/optgen.cc`
*   **Purpose:** Simulates Belady's OPT algorithm on past memory accesses to determine if a past access should have been a cache hit based on cache demand.
*   **Implementation:** 
    *   Maintains an `occupancy` vector for each cache set with a capacity of $8 \times \text{associativity}$.
    *   Records the `lastAccess` timestamp for addresses in a hash map.
    *   When an address is reused, it checks the interval between the current time and the last access. If the occupancy count never exceeded the cache associativity during that window, the access is marked as a **HIT** (meaning OPT would have retained it), and the occupancy vector for that interval is incremented.

### 2. Hawkeye Predictor: Learning PC Behavior
**Files:** `ChampSim/replacement/hawkeye/predictor.h`, `ChampSim/replacement/hawkeye/predictor.cc`
*   **Purpose:** A Program Counter (PC)-indexed predictor that learns whether specific load instructions cause cache-friendly or cache-averse memory accesses.
*   **Implementation:**
    *   Utilizes an 8192-entry table of 3-bit saturating counters (values 0-7).
    *   **Hashing:** Indexes the table using `(pc ^ (pc >> 12)) % num_entries`.
    *   **Training (`train`):** Increments the counter on an OPTgen hit (moving toward cache-friendly) and decrements it on a miss (moving toward cache-averse).
    *   **Prediction (`predict`):** Predicts "cache-friendly" if the counter is $\geq 4$ (the threshold value).

### 3. RRIP: Insertion and Aging Logic
**Files:** `ChampSim/replacement/hawkeye/rrip.h`, `ChampSim/replacement/hawkeye/rrip.cc`
*   **Purpose:** Manages the cache replacement state using Re-Reference Interval Prediction (RRIP) values based on the predictor's classification.
*   **Implementation:**
    *   **Insertion (`update_rrpv`):** Cache-friendly lines are inserted with the highest priority (`RRPV = 0`). Cache-averse lines are inserted with the lowest priority (`RRPV = 7`).
    *   **Victim Selection (`find_victim`):** Scans the set for a line with `RRPV == 7`. If none exists, it ages all lines in the set by incrementing their RRPV until a victim (7) is found.

### 4. The ChampSim Adapter
**Files:** `ChampSim/replacement/hawkeye/hawkeye.h`, `ChampSim/replacement/hawkeye/hawkeye.cc`
*   **Purpose:** Connects ChampSim's internal cache framework to the standalone Hawkeye components.
*   **Implementation:**
    *   Inherits from `champsim::modules::replacement`.
    *   Instantiates `OPTgen`, `HawkeyePredictor`, and a 2D `rrpv` vector.
    *   Delegates ChampSim's `update_replacement_state` hook to `optgen.access()` and `predictor.train()`.
    *   Delegates the `replacement_cache_fill` hook to `predictor.predict()` and `update_rrpv()`.
    *   Delegates `find_victim` to the standalone RRIP victim selection logic.

### 5. Standalone Testing
**Files:** `ChampSim/hawkeye_tests/optgen_test.cc`, `ChampSim/hawkeye_tests/predictor_test.cc`, `ChampSim/hawkeye_tests/rrip_test.cc`
*   **Purpose:** Standalone C++ test drivers used to independently validate the logic, state changes, and print expected outputs for automated grading, completely decoupled from the ChampSim build environment.

### 6. Automation & Plotting Scripts
**Files:** `ChampSim/run_exp1.sh`, `ChampSim/run_exp2.sh`, `ChampSim/generate_plots.py`
*   **Purpose:** Bash scripts used to automate the configuration, compilation, and execution of the ChampSim experiments. A Python script is included to parse the raw outputs and generate the final visualization graphs (`plot1_associativity.png` and `plot2_reduction.png`).

---

## Complete Build and Run Instructions

The evaluation requires running both `lru` and `hawkeye` policies across different associativity setups and traces. All runs use a 20 Million instruction warmup and a 50 Million instruction simulation phase.

**Important:** Before running any commands, navigate into the `ChampSim` directory:
```bash
cd ChampSim
```

*(Note: Ensure your JSON configuration files—e.g., `hawkeye_4way.json`, `lru_16way.json`—are properly configured with the correct `sets` and `ways` to maintain a fixed 2MB LLC capacity before running.)*

### Experiment 1: LLC Miss Rate vs. Associativity
This experiment tests the `456.hmmer-191B.champsimtrace.xz` trace across 4-way, 8-way, and 16-way LLC configurations.

**1. Run 4-Way Configurations (Sets: 8192, Ways: 4):**
```bash
# LRU 4-way
./config.sh lru_4way.json
make -j$(nproc)
bin/champsim --warmup_instructions 20000000 --simulation_instructions 50000000 traces/456.hmmer-191B.champsimtrace.xz

# Hawkeye 4-way
./config.sh hawkeye_4way.json
make -j$(nproc)
bin/champsim --warmup_instructions 20000000 --simulation_instructions 50000000 traces/456.hmmer-191B.champsimtrace.xz
```

**2. Run 8-Way Configurations (Sets: 4096, Ways: 8):**
```bash
# LRU 8-way
./config.sh lru_8way.json
make -j$(nproc)
bin/champsim --warmup_instructions 20000000 --simulation_instructions 50000000 traces/456.hmmer-191B.champsimtrace.xz

# Hawkeye 8-way
./config.sh hawkeye_8way.json
make -j$(nproc)
bin/champsim --warmup_instructions 20000000 --simulation_instructions 50000000 traces/456.hmmer-191B.champsimtrace.xz
```

**3. Run 16-Way Configurations (Sets: 2048, Ways: 16):**
```bash
# LRU 16-way
./config.sh lru_16way.json
make -j$(nproc)
bin/champsim --warmup_instructions 20000000 --simulation_instructions 50000000 traces/456.hmmer-191B.champsimtrace.xz

# Hawkeye 16-way
./config.sh hawkeye_16way.json
make -j$(nproc)
bin/champsim --warmup_instructions 20000000 --simulation_instructions 50000000 traces/456.hmmer-191B.champsimtrace.xz
```

### Experiment 2: Miss-Rate Reduction over LRU Across Benchmarks
This experiment uses the default 16-way 2MB LLC configuration across three different benchmarks to compare LRU vs. Hawkeye.

**1. Compile the Baseline (LRU 16-way) and run all 3 traces:**
```bash
./config.sh lru_16way.json
make -j$(nproc)

bin/champsim --warmup_instructions 20000000 --simulation_instructions 50000000 traces/456.hmmer-191B.champsimtrace.xz
bin/champsim --warmup_instructions 20000000 --simulation_instructions 50000000 traces/429.mcf-22B.champsimtrace.xz
bin/champsim --warmup_instructions 20000000 --simulation_instructions 50000000 traces/473.astar-42B.champsimtrace.xz
```

**2. Compile the Proposed Policy (Hawkeye 16-way) and run all 3 traces:**
```bash
./config.sh hawkeye_16way.json
make -j$(nproc)

bin/champsim --warmup_instructions 20000000 --simulation_instructions 50000000 traces/456.hmmer-191B.champsimtrace.xz
bin/champsim --warmup_instructions 20000000 --simulation_instructions 50000000 traces/429.mcf-22B.champsimtrace.xz
bin/champsim --warmup_instructions 20000000 --simulation_instructions 50000000 traces/473.astar-42B.champsimtrace.xz
```