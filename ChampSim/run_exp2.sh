#!/bin/bash

WARMUP=20000000
SIM=50000000
# Ensure you use your 16-way default configs here
LRU_CONFIG="lru_16way" 
HAWK_CONFIG="hawkeye_16way"

TRACES=(
    "456.hmmer-191B"
    "429.mcf-22B"
    "473.astar-42B"
)
RESULTS_DIR="results_exp2"
PLOT_FILE="plot2_data.csv"

mkdir -p "$RESULTS_DIR"
echo "Benchmark,LRU_MissRate(%),Hawkeye_MissRate(%),Reduction(%)" > "$PLOT_FILE"

# Function to run simulation and extract miss rate
run_and_extract() {
    local config=$1
    local trace_path="traces/${2}.champsimtrace.xz"
    local out_file="$RESULTS_DIR/${config}_${2}_output.txt"
    
    bin/champsim --warmup_instructions $WARMUP --simulation_instructions $SIM "$trace_path" > "$out_file"
    
    awk '/LLC TOTAL.*ACCESS:/ {
        for(i=1; i<=NF; i++) {
            if($i=="ACCESS:") acc=$(i+1);
            if($i=="MISS:") mss=$(i+1);
        }
    } END { 
        if(acc>0) printf "%.2f", (mss/acc)*100; else print "0";
    }' "$out_file"
}

# Compile both policies once
echo "Building LRU..."
./config.sh "${LRU_CONFIG}.json"
make -j$(nproc)
cp bin/champsim bin/champsim_lru

echo "Building Hawkeye..."
./config.sh "${HAWK_CONFIG}.json"
make -j$(nproc)
cp bin/champsim bin/champsim_hawk

for trace in "${TRACES[@]}"; do
    echo "Running LRU on ${trace}..."
    cp bin/champsim_lru bin/champsim
    lru_mr=$(run_and_extract "$LRU_CONFIG" "$trace")
    
    echo "Running Hawkeye on ${trace}..."
    cp bin/champsim_hawk bin/champsim
    hawk_mr=$(run_and_extract "$HAWK_CONFIG" "$trace")
    
    # Calculate reduction: (LRU - Hawk) / LRU * 100
    reduction=$(awk -v lru="$lru_mr" -v hawk="$hawk_mr" 'BEGIN { 
        if (lru > 0) printf "%.2f", ((lru - hawk) / lru) * 100; else print "0"; 
    }')
    
    # Save directly for plotting
    echo "${trace},${lru_mr},${hawk_mr},${reduction}" >> "$PLOT_FILE"
    echo "Completed ${trace}: Reduction = ${reduction}%"
done

echo "Experiment 2 complete. Ready-to-plot data saved in $PLOT_FILE."