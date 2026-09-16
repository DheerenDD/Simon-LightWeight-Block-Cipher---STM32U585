#!/usr/bin/env python3
"""
Regenerate the benchmark figures from benchmark_data.csv.

Usage:
    pip install matplotlib numpy
    python plot_results.py

Writes fig1..fig4 PNGs into ../docs/figures/.
"""
import csv
import os
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import numpy as np

HERE = os.path.dirname(os.path.abspath(__file__))
OUT = os.path.join(HERE, "..", "docs", "figures")
os.makedirs(OUT, exist_ok=True)

FCLK = 160e6
NAVY, BLUE, RED, ORANGE = "#1f4e79", "#2e75b6", "#c00000", "#ed7d31"


def load():
    rows = []
    with open(os.path.join(HERE, "benchmark_data.csv")) as f:
        for r in csv.DictReader(f):
            rows.append(r)
    return rows


def series(rows, cipher, mode, field):
    sel = [r for r in rows if r["cipher"] == cipher and r["mode"] == mode]
    sel.sort(key=lambda r: int(r["blocks"]))
    blocks = [int(r["blocks"]) for r in sel]
    vals = [float(r[field]) for r in sel]
    return blocks, vals


def main():
    rows = load()
    blocks = [1, 2, 4, 8, 16, 32, 64]

    se_cpb = series(rows, "Simon64/128", "ECB", "cycles_per_byte")[1]
    sc_cpb = series(rows, "Simon64/128", "CBC", "cycles_per_byte")[1]
    ae_cpb = series(rows, "AES-128", "ECB", "cycles_per_byte")[1]
    ac_cpb = series(rows, "AES-128", "CBC", "cycles_per_byte")[1]

    se_cyc = series(rows, "Simon64/128", "ECB", "cycles")[1]
    sc_cyc = series(rows, "Simon64/128", "CBC", "cycles")[1]
    ae_cyc = series(rows, "AES-128", "ECB", "cycles")[1]
    ac_cyc = series(rows, "AES-128", "CBC", "cycles")[1]

    def us(c):
        return [x / FCLK * 1e6 for x in c]

    # Fig 1: cycles/byte
    plt.figure(figsize=(7, 4.5))
    plt.plot(blocks, se_cpb, "o-", color=NAVY, label="Simon64/128 ECB (SW)")
    plt.plot(blocks, sc_cpb, "s--", color=BLUE, label="Simon64/128 CBC (SW)")
    plt.plot(blocks, ae_cpb, "^-", color=RED, label="AES-128 ECB (HW)")
    plt.plot(blocks, ac_cpb, "v--", color=ORANGE, label="AES-128 CBC (HW)")
    plt.xscale("log", base=2); plt.xticks(blocks, blocks)
    plt.xlabel("Number of blocks"); plt.ylabel("Cycles per byte")
    plt.title("Efficiency: Cycles per Byte vs Data Size")
    plt.legend(); plt.grid(True, alpha=0.3); plt.tight_layout()
    plt.savefig(os.path.join(OUT, "fig1_cpb.png"), dpi=150); plt.close()

    # Fig 2: time log-log
    plt.figure(figsize=(7, 4.5))
    plt.plot(blocks, us(se_cyc), "o-", color=NAVY, label="Simon64/128 ECB (SW)")
    plt.plot(blocks, us(sc_cyc), "s--", color=BLUE, label="Simon64/128 CBC (SW)")
    plt.plot(blocks, us(ae_cyc), "^-", color=RED, label="AES-128 ECB (HW)")
    plt.plot(blocks, us(ac_cyc), "v--", color=ORANGE, label="AES-128 CBC (HW)")
    plt.xscale("log", base=2); plt.yscale("log"); plt.xticks(blocks, blocks)
    plt.xlabel("Number of blocks"); plt.ylabel("Encryption time (microseconds)")
    plt.title("Encryption Time vs Data Size (log-log)")
    plt.legend(); plt.grid(True, alpha=0.3, which="both"); plt.tight_layout()
    plt.savefig(os.path.join(OUT, "fig2_time.png"), dpi=150); plt.close()

    # Fig 3: absolute cycles bar
    plt.figure(figsize=(7.2, 4.5))
    x = np.arange(len(blocks)); w = 0.2
    plt.bar(x - 1.5 * w, se_cyc, w, color=NAVY, label="Simon ECB (SW)")
    plt.bar(x - 0.5 * w, sc_cyc, w, color=BLUE, label="Simon CBC (SW)")
    plt.bar(x + 0.5 * w, ae_cyc, w, color=RED, label="AES ECB (HW)")
    plt.bar(x + 1.5 * w, ac_cyc, w, color=ORANGE, label="AES CBC (HW)")
    plt.xticks(x, blocks); plt.xlabel("Number of blocks"); plt.ylabel("CPU cycles")
    plt.title("Absolute Cycle Cost vs Data Size")
    plt.legend(); plt.grid(True, alpha=0.3, axis="y"); plt.tight_layout()
    plt.savefig(os.path.join(OUT, "fig3_cycles.png"), dpi=150); plt.close()

    # Fig 4: cross-platform (from reference paper, normalized to cyc/byte)
    plats = ["STM32F103\n(M3,64MHz)", "STM32L476\n(M4,80MHz)",
             "STM32F429\n(M4,180MHz)", "STM32U585\n(M33,160MHz)"]
    cpb = [480.0, 600.0, 765.0, 211.1]
    colors = ["#7f7f7f", "#7f7f7f", "#7f7f7f", NAVY]
    plt.figure(figsize=(7, 4.5))
    bars = plt.bar(plats, cpb, color=colors)
    for b, v in zip(bars, cpb):
        plt.text(b.get_x() + b.get_width() / 2, v + 8, f"{v:.0f}",
                 ha="center", fontsize=9)
    plt.ylabel("Cycles per byte (Simon64/128)")
    plt.title("Cross-Platform Simon64/128 Efficiency\n(this work vs El Hanine et al. 2025)")
    plt.grid(True, alpha=0.3, axis="y"); plt.tight_layout()
    plt.savefig(os.path.join(OUT, "fig4_crossplat.png"), dpi=150); plt.close()

    print("Figures written to", os.path.normpath(OUT))


if __name__ == "__main__":
    main()
