#!/bin/bash

TRACE="traces/456.hmmer-191B.champsimtrace.xz"
WARMUP=20000000
SIM=50000000
WAYS=(4 8 16)
RESULTS_DIR="results_exp1"
PLOT_FILE="plot1_data.csv"

mkdir -p "$RESULTS_DIR"
echo "Associativity,LRU_MissRate(%),Hawkeye_MissRate(%)" > "$PLOT_FILE"

# Function to run simulation and extract miss rate
run_and_extract() {
    local config=$1
    local out_file="$RESULTS_DIR/${config}_output.txt"
    
    ./config.sh "${config}.json"
    make -j$(nproc)
    bin/champsim --warmup_instructions $WARMUP --simulation_instructions $SIM "$TRACE" > "$out_file"
    
    awk '/LLC TOTAL.*ACCESS:/ {
        for(i=1; i<=NF; i++) {
            if($i=="ACCESS:") acc=$(i+1);
            if($i=="MISS:") mss=$(i+1);
        }
    } END { 
        if(acc>0) printf "%.2f", (mss/acc)*100; else print "0";
    }' "$out_file"
}

for way in "${WAYS[@]}"; do
    echo "Running LRU ${way}-way..."
    lru_mr=$(run_and_extract "lru_${way}way")
    
    echo "Running Hawkeye ${way}-way..."
    hawk_mr=$(run_and_extract "hawkeye_${way}way")
    
    # Save directly for plotting
    echo "${way},${lru_mr},${hawk_mr}" >> "$PLOT_FILE"
    echo "Completed ${way}-way: LRU = ${lru_mr}%, Hawkeye = ${hawk_mr}%"
done

echo "Experiment 1 complete. Ready-to-plot data saved in $PLOT_FILE."