import os
import subprocess
import math
import tempfile
import numpy as np
import matplotlib.pyplot as plt
import re

# --- configuration ---
MAX_PROCS = 6
# processor lists: bitonic uses powers of two, others use all cores 1..MAX_PROCS
BITONIC_PROCS = [p for p in [1,2,4] if p <= MAX_PROCS]
OTHER_PROCS = list(range(1, MAX_PROCS+1))
SIZES = [1_000, 10_000, 100_000, 1_000_000]
EXE = "./parallel_computation.o"

# directories for inputs and graphs
IMG_DIR = "docs/imgs"
INPUT_DIR = "docs/input"
os.makedirs(IMG_DIR, exist_ok=True)
os.makedirs(INPUT_DIR, exist_ok=True)

# list of algorithms: name and menu choice
ALGOS = [
    {"name": "Quick Search",      "choice": "1"},
    {"name": "Prime Number Finding", "choice": "2"},
    {"name": "Bitonic Sort",      "choice": "3"},
    {"name": "Radix Sort",        "choice": "4"},
    {"name": "Sample Sort",       "choice": "5"},
]

# --- helper to round up to next power of two ---
def next_pow2(n):
    return 1<<(n-1).bit_length()

# --- compile once ---
print("Compiling project…")
subprocess.run(["bash", "compile.sh"], check=True)

# --- run experiments per algorithm ---
N_TRIALS = 5  # number of runs per config to average

for algo in ALGOS:
    name = algo["name"]
    choice = algo["choice"]
    results = {}
    for size in SIZES:
        # determine parameters
        if choice in ["1", "3", "4", "5"]:
            N = next_pow2(size)
            # generate random data file under docs/input
            datafile = f"{INPUT_DIR}/input_{choice}_{N}.txt"
            arr = np.random.randint(0, N, size=N)
            with open(datafile, "w") as f:
                f.write(" ".join(map(str, arr)))
        else:
            # prime range
            lower, upper = 1, size
            N = size
        results[N] = {}
        # choose processor set per algorithm: Bitonic and Sample use powers of two
        procs = BITONIC_PROCS if choice in ["3", "5"] else OTHER_PROCS
        for p in procs:
            # run multiple trials and compute average time
            times_list = []
            for trial in range(N_TRIALS):
                cmd = ["mpirun", "-np", str(p), EXE]
                # build stdin for program
                if choice == "1":
                    target = np.random.randint(0, N)
                    inp = "\n".join([choice, datafile, str(target), "N"])
                elif choice == "2":
                    inp = "\n".join([choice, str(lower), str(upper), "N"])
                else:
                    inp = "\n".join([choice, datafile, "N"])
                proc = subprocess.run(cmd, input=inp.encode(), stdout=subprocess.PIPE, stderr=subprocess.PIPE, check=True)
                out = proc.stdout.decode()
                # parse 'Time Taken: <seconds>' using regex
                t = None
                for line in out.splitlines():
                    m = re.search(r"Time Taken:\s*([0-9]*\.?[0-9]+)", line)
                    if m:
                        t = float(m.group(1))
                        break
                if t is None:
                    raise RuntimeError(f"could not parse time from output:\n{out}")
                times_list.append(t)
            avg_t = sum(times_list) / len(times_list)
            results[N][p] = avg_t
            print(f"{name}: size={N}, p={p}, avg over {N_TRIALS} runs → {avg_t:.6f}s")

    # plot results per algorithm
    for N, times in results.items():
        ps = sorted(times)
        t1 = times.get(1)
        ts = np.array([times[p] for p in ps])
        speedup = t1 / ts
        efficiency = speedup / np.array(ps)
        # always produce separate time and speedup/efficiency graphs
        name_key = name.replace(' ', '_').lower()
        # Time graph
        fig, ax = plt.subplots()
        ax.plot(ps, ts, marker='o', color='C0')
        ax.set_xlabel("processes")
        ax.set_ylabel("time (s)")
        # log-scale for power-of-two cases
        if choice in ["3", "5"]:
            ax.set_xscale('log', base=2)
            ax.set_xticks(ps)
            ax.get_xaxis().set_major_formatter(plt.ScalarFormatter())
        ax.set_title(f"{name} Time (N={N})")
        plt.tight_layout()
        time_png = f"{IMG_DIR}/{name_key}_time_{N}.png"
        plt.savefig(time_png)
        print(f"Saved time graph: {time_png}")
        # Speedup & Efficiency graph
        fig2, ax2 = plt.subplots()
        ax2.plot(ps, speedup, marker='s', linestyle='--', color='C1', label="speedup")
        ax2.plot(ps, efficiency, marker='^', linestyle=':', color='C2', label="efficiency")
        ax2.set_xlabel("processes")
        ax2.set_ylabel("value")
        if choice in ["3", "5"]:
            ax2.set_xscale('log', base=2)
            ax2.set_xticks(ps)
            ax2.get_xaxis().set_major_formatter(plt.ScalarFormatter())
        ax2.legend(loc="best")
        ax2.set_title(f"{name} Speedup & Efficiency (N={N})")
        plt.tight_layout()
        perf_png = f"{IMG_DIR}/{name_key}_perf_{N}.png"
        plt.savefig(perf_png)
        print(f"Saved speedup/eff graph: {perf_png}")