import os
import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

def extract_miss_rate(filepath):
    """Extracts the final simulation LLC Miss Rate from a ChampSim output text file."""
    try:
        with open(filepath, 'r') as f:
            lines = f.readlines()
        
        # Read from bottom up to ensure we get the Simulation phase, not Warmup
        for line in reversed(lines):
            if 'LLC TOTAL' in line and 'ACCESS:' in line:
                parts = line.split()
                try:
                    acc_idx = parts.index('ACCESS:') + 1
                    mss_idx = parts.index('MISS:') + 1
                    acc = float(parts[acc_idx])
                    mss = float(parts[mss_idx])
                    if acc > 0:
                        return (mss / acc) * 100
                except (ValueError, IndexError):
                    pass
                break
    except Exception as e:
        print(f"Warning: Could not read {filepath}")
    return 0.0

# --- Process Experiment 1 ---
ways = [4, 8, 16]
lru_mr_1 = [extract_miss_rate(f"results_exp1/lru_{w}way_output.txt") for w in ways]
hawk_mr_1 = [extract_miss_rate(f"results_exp1/hawkeye_{w}way_output.txt") for w in ways]

df1 = pd.DataFrame({
    "Associativity": ways,
    "LRU_MissRate(%)": lru_mr_1,
    "Hawkeye_MissRate(%)": hawk_mr_1
})
print("--- Experiment 1 Data ---")
print(df1)
df1.to_csv("plot1_data_fixed.csv", index=False)

x = np.arange(len(df1["Associativity"]))
width = 0.35
fig, ax1 = plt.subplots(figsize=(8, 6))
ax1.bar(x - width/2, df1["LRU_MissRate(%)"], width, label="LRU", color="#1f77b4")
ax1.bar(x + width/2, df1["Hawkeye_MissRate(%)"], width, label="Hawkeye", color="#ff7f0e")

ax1.set_xlabel("LLC Associativity (Ways)")
ax1.set_ylabel("LLC Miss Rate (%)")
ax1.set_title("LLC Miss Rate vs. Associativity (456.hmmer-191B)")
ax1.set_xticks(x)
ax1.set_xticklabels(df1["Associativity"])
ax1.legend()
plt.tight_layout()
plt.savefig("plot1_associativity.png", dpi=300)
plt.close()

# --- Process Experiment 2 ---
traces = ["456.hmmer-191B", "429.mcf-22B", "473.astar-42B"]
lru_mr_2 = [extract_miss_rate(f"results_exp2/lru_16way_{t}_output.txt") for t in traces]
hawk_mr_2 = [extract_miss_rate(f"results_exp2/hawkeye_16way_{t}_output.txt") for t in traces]
reductions = [((l - h) / l * 100) if l > 0 else 0 for l, h in zip(lru_mr_2, hawk_mr_2)]

df2 = pd.DataFrame({
    "Benchmark": traces,
    "LRU_MissRate(%)": lru_mr_2,
    "Hawkeye_MissRate(%)": hawk_mr_2,
    "Reduction(%)": reductions
})
print("\n--- Experiment 2 Data ---")
print(df2)
df2.to_csv("plot2_data_fixed.csv", index=False)

fig, ax2 = plt.subplots(figsize=(8, 6))
bars = ax2.bar(df2["Benchmark"], df2["Reduction(%)"], color="#2ca02c")
ax2.set_xlabel("Benchmark")
ax2.set_ylabel("LLC Miss-Rate Reduction (%) over LRU")
ax2.set_title("Hawkeye Miss-Rate Reduction vs LRU (16-way, 2MB)")
ax2.axhline(0, color='black', linewidth=1)

for bar in bars:
    yval = bar.get_height()
    offset = max(1.0, yval * 0.05) if yval >= 0 else -max(1.0, abs(yval) * 0.05)
    ax2.text(bar.get_x() + bar.get_width()/2, yval + offset, f"{yval:.2f}%", ha="center", va="bottom")

plt.tight_layout()
plt.savefig("plot2_reduction.png", dpi=300)
plt.close()

print("\nSuccess! Clean data saved to plot1_data_fixed.csv and plot2_data_fixed.csv.")
print("Graphs saved as plot1_associativity.png and plot2_reduction.png.")