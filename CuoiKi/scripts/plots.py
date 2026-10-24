# -*- coding: utf-8 -*-
"""Tổng hợp CSV trong results/raw/ -> bảng trung vị + biểu đồ PDF cho báo cáo/slide.

Chạy:  python scripts/plots.py
Đầu ra:
    results/summary/*.csv       bảng trung vị
    report/figures/*.pdf|png    biểu đồ (dùng chung cho báo cáo và slide)
    report/generated/*.tex      bảng LaTeX sinh tự động
"""
import os

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt  # noqa: E402
import pandas as pd  # noqa: E402

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
RAW = os.path.join(ROOT, "results", "raw")
SUM = os.path.join(ROOT, "results", "summary")
FIG = os.path.join(ROOT, "report", "figures")
GEN = os.path.join(ROOT, "report", "generated")
for d in (SUM, FIG, GEN):
    os.makedirs(d, exist_ok=True)

ELEM = 8                 # byte / phần tử (int64)
BYTES_MIN = 2 * ELEM     # tối thiểu: đọc 1 lần + ghi 1 lần

LABEL = {"seq": "Tuần tự", "hillis_steele": "Hillis–Steele", "blelloch": "Blelloch",
         "blocked": "Blocked", "blocked_opt": "Blocked-opt", "omp_scan": "omp scan",
         "copy": "Copy (mốc băng thông)"}
COLOR = {"seq": "#000000", "hillis_steele": "#d62728", "blelloch": "#ff7f0e",
         "blocked": "#2ca02c", "blocked_opt": "#1f4e79", "omp_scan": "#9467bd",
         "copy": "#7f7f7f"}
MARK = {"seq": "o", "hillis_steele": "v", "blelloch": "s", "blocked": "^",
        "blocked_opt": "D", "omp_scan": "P", "copy": "x"}
ORDER = ["seq", "hillis_steele", "blelloch", "blocked", "blocked_opt", "omp_scan", "copy"]

plt.rcParams.update({
    "font.family": "DejaVu Sans", "font.size": 10, "axes.grid": True, "grid.alpha": 0.3,
    "axes.spines.top": False, "axes.spines.right": False, "legend.frameon": False,
    "figure.dpi": 150, "savefig.bbox": "tight",
})


def load(name):
    path = os.path.join(RAW, name)
    if not os.path.exists(path):
        print("  (bo qua, khong co)", name)
        return None
    return pd.read_csv(path)


def median(df, keys):
    g = df.groupby(keys)["time_s"]
    out = g.median().rename("t_med").to_frame()
    out["t_min"] = g.min()
    out["t_max"] = g.max()
    out["reps"] = g.count()
    return out.reset_index()


def save(fig, name):
    fig.savefig(os.path.join(FIG, name + ".pdf"))
    fig.savefig(os.path.join(FIG, name + ".png"), dpi=200)
    plt.close(fig)
    print("  ->", name)


def algos_in(df):
    return [a for a in ORDER if a in set(df["algo"])]


def top_legend(fig, ax, ncol=4):
    fig.legend(*ax.get_legend_handles_labels(), loc="upper center", ncol=ncol,
               bbox_to_anchor=(0.5, 1.08))


# ---------------------------------------------------------------- 1. theo n
def fig_size():
    df = load("size.csv")
    if df is None:
        return
    m = median(df, ["algo", "n", "threads"])
    m.to_csv(os.path.join(SUM, "size.csv"), index=False)

    fig, ax = plt.subplots(1, 2, figsize=(10, 3.8))
    for a in algos_in(m):
        p = 1 if a == "seq" else 12
        s = m[(m.algo == a) & (m.threads == p)].sort_values("n")
        if s.empty:
            continue
        lab = LABEL[a] + ("" if a == "seq" else " (p=12)")
        ax[0].plot(s.n, s.t_med * 1e3, marker=MARK[a], color=COLOR[a], ms=4, label=lab)
        ax[1].plot(s.n, s.t_med / s.n * 1e9, marker=MARK[a], color=COLOR[a], ms=4)
    l3 = 12 * 2**20 / (2 * ELEM)   # n tại đó in+out = 12 MiB
    for x in ax:
        x.set_xscale("log", base=2)
        x.set_yscale("log")
        x.set_xlabel("Kích thước mảng n")
        x.axvline(l3, color="gray", ls=":", lw=1)
    ax[1].text(l3 * 1.1, ax[1].get_ylim()[1] * 0.5, "in+out = L3", fontsize=8, color="gray")
    ax[0].set_ylabel("Thời gian (ms)")
    ax[0].set_title("(a) Thời gian chạy")
    ax[1].set_ylabel("ns / phần tử")
    ax[1].set_title("(b) Thời gian trên mỗi phần tử")
    top_legend(fig, ax[0])
    save(fig, "size_time")

    seq = m[m.algo == "seq"].set_index("n").t_med
    fig, ax = plt.subplots(figsize=(6, 3.6))
    for a in algos_in(m):
        if a in ("seq", "copy"):
            continue
        s = m[(m.algo == a) & (m.threads == 12)].set_index("n").t_med
        sp = (seq / s).dropna()
        ax.plot(sp.index, sp.values, marker=MARK[a], color=COLOR[a], ms=4, label=LABEL[a])
    ax.axhline(1, color="black", lw=0.8)
    ax.axvline(l3, color="gray", ls=":", lw=1)
    ax.set_xscale("log", base=2)
    ax.set_yscale("log", base=2)
    ax.set_xlabel("Kích thước mảng n")
    ax.set_ylabel("Speedup so với tuần tự (p = 12)")
    ax.legend(ncol=2, fontsize=8)
    save(fig, "size_speedup")


# ---------------------------------------------------------------- 2. strong scaling
def fig_strong(fname="strong.csv", tag="strong"):
    df = load(fname)
    if df is None:
        return None
    m = median(df, ["algo", "n", "threads"])
    n = int(m.n.iloc[0])
    t_seq = float(m[m.algo == "seq"].t_med.iloc[0])
    m["speedup"] = t_seq / m.t_med
    m["efficiency"] = m.speedup / m.threads
    m["gbps"] = BYTES_MIN * n / m.t_med / 1e9
    m.to_csv(os.path.join(SUM, f"{tag}.csv"), index=False)
    ps = sorted(m.threads.unique())

    fig, ax = plt.subplots(1, 2, figsize=(10, 3.8))
    for a in algos_in(m):
        if a == "seq":
            continue
        s = m[m.algo == a].sort_values("threads")
        ax[0].plot(s.threads, s.speedup, marker=MARK[a], color=COLOR[a], ms=4, label=LABEL[a],
                   ls="--" if a == "copy" else "-")
        if a != "copy":
            ax[1].plot(s.threads, s.efficiency, marker=MARK[a], color=COLOR[a], ms=4)
    ax[0].plot(ps, ps, "k:", lw=0.8, label="Lý tưởng")
    ax[0].axhline(1, color="black", lw=0.6)
    ax[0].set_ylim(0, max(4.5, m[m.algo != "seq"].speedup.max() * 1.15))
    ax[0].set_ylabel("Speedup so với tuần tự")
    ax[0].set_title(f"(a) Speedup, n = 2^{n.bit_length() - 1}")
    ax[1].set_ylabel("Hiệu suất E = S / p")
    ax[1].set_title("(b) Hiệu suất song song")
    ax[1].set_ylim(0, None)
    for x in ax:
        x.set_xlabel("Số luồng p")
        x.set_xticks(ps)
    top_legend(fig, ax[0])
    save(fig, f"{tag}_speedup")

    fig, ax = plt.subplots(figsize=(6, 3.6))
    for a in algos_in(m):
        if a == "hillis_steele":
            continue
        s = m[m.algo == a].sort_values("threads")
        ax.plot(s.threads, s.gbps, marker=MARK[a], color=COLOR[a], ms=4, label=LABEL[a],
                ls="--" if a == "copy" else "-")
    ax.set_xlabel("Số luồng p")
    ax.set_ylabel("Băng thông hiệu dụng (GB/s)\n= 16 B × n / thời gian")
    ax.set_xticks(ps)
    ax.legend(fontsize=8, ncol=2)
    save(fig, f"{tag}_bandwidth")
    return m


# ---------------------------------------------------------------- 3. weak scaling
def fig_weak():
    df = load("weak.csv")
    if df is None:
        return
    m = median(df, ["algo", "n", "threads"])
    m.to_csv(os.path.join(SUM, "weak.csv"), index=False)
    fig, ax = plt.subplots(figsize=(6, 3.6))
    for a in algos_in(m):
        s = m[m.algo == a].sort_values("threads")
        base = float(s.t_med.iloc[0])
        ax.plot(s.threads, base / s.t_med, marker=MARK[a], color=COLOR[a], ms=4,
                label=LABEL[a], ls="--" if a == "copy" else "-")
    ax.axhline(1, color="black", lw=0.8, ls=":")
    ax.set_xscale("log", base=2)
    ax.set_xticks([1, 2, 4, 8])
    ax.set_xticklabels(["1", "2", "4", "8"])
    ax.set_xlabel("Số luồng p   (n = p × 2^23)")
    ax.set_ylabel("Hiệu suất weak scaling  T(1) / T(p)")
    ax.set_ylim(0, 1.15)
    ax.legend(fontsize=8)
    save(fig, "weak")


# ---------------------------------------------------------------- 4. block size
def fig_block():
    df = load("block.csv")
    if df is None:
        return
    m = median(df, ["algo", "n", "threads", "block"])
    m.to_csv(os.path.join(SUM, "block.csv"), index=False)
    fig, ax = plt.subplots(figsize=(6, 3.4))
    kib = m.block * ELEM / 1024
    ax.plot(kib, m.t_med * 1e3, marker="D", color=COLOR["blocked_opt"], ms=4)
    ax.set_xscale("log", base=2)
    ax.set_xlabel("Kích thước khối B mỗi luồng (KiB)")
    ax.set_ylabel("Thời gian (ms), n = 2^26, p = 12")
    top = ax.get_ylim()[1]
    for c, lab in ((48, "L1d"), (1280, "L2 (P-core)"), (12 * 1024, "L3")):
        ax.axvline(c, color="gray", ls=":", lw=1)
        ax.text(c * 1.07, top, lab, fontsize=7, color="gray", va="top")
    save(fig, "block_size")


# ---------------------------------------------------------------- 5. bind
def fig_bind(t_seq):
    df = load("bind.csv")
    if df is None or not t_seq:
        return
    m = median(df, ["bind", "algo", "n", "threads"])
    m["speedup"] = t_seq / m.t_med
    m.to_csv(os.path.join(SUM, "bind.csv"), index=False)
    fig, ax = plt.subplots(figsize=(6, 3.6))
    ls = {"false": "-", "close": "--", "spread": ":"}
    for a in ("blocked", "blocked_opt"):
        for b in ("false", "close", "spread"):
            s = m[(m.algo == a) & (m.bind == b)].sort_values("threads")
            if not s.empty:
                ax.plot(s.threads, s.speedup, ls=ls[b], marker=MARK[a], color=COLOR[a], ms=4,
                        label=f"{LABEL[a]}, bind={b}")
    ax.set_xlabel("Số luồng p")
    ax.set_ylabel("Speedup so với tuần tự")
    ax.set_xticks(sorted(m.threads.unique()))
    ax.legend(fontsize=7, ncol=2)
    save(fig, "bind")


# ---------------------------------------------------------------- 6. compaction
def fig_compact():
    path = os.path.join(RAW, "compact.csv")
    if not os.path.exists(path):
        return
    df = pd.read_csv(path)
    m = df.groupby(["version", "threads"])["time_s"].median().reset_index()
    t_seq = float(m[m.version == "seq"].time_s.iloc[0])
    m["speedup"] = t_seq / m.time_s
    m.to_csv(os.path.join(SUM, "compact.csv"), index=False)
    fig, ax = plt.subplots(figsize=(6, 3.4))
    ps = sorted(m[m.version != "seq"].threads.unique())
    w = 0.38
    for k, (v, lab, col) in enumerate((("par3", "3 lượt (flag + scan + ghi)", "#9fb8d0"),
                                       ("fused", "Hợp nhất (đếm + ghi)", COLOR["blocked_opt"]))):
        s = m[m.version == v].set_index("threads").reindex(ps)
        xs = [i + (k - 0.5) * w for i in range(len(ps))]
        ax.bar(xs, s.speedup, w, color=col, label=lab)
        for x, y in zip(xs, s.speedup):
            ax.text(x, y + 0.04, f"{y:.2f}", ha="center", fontsize=7)
    ax.axhline(1, color="black", lw=0.8)
    ax.set_xticks(range(len(ps)))
    ax.set_xticklabels([str(p) for p in ps])
    ax.set_xlabel("Số luồng p")
    ax.set_ylabel("Speedup so với lọc tuần tự")
    ax.legend(fontsize=8)
    save(fig, "compact")


# ---------------------------------------------------------------- bảng LaTeX
def tex_strong(m):
    if m is None:
        return
    algos = [a for a in algos_in(m) if a != "seq"]
    ps = [p for p in sorted(m.threads.unique()) if p in (1, 2, 4, 8, 12)]
    t_seq = float(m[m.algo == "seq"].t_med.iloc[0])
    lines = [r"\begin{tabular}{l" + "r" * len(ps) + "}", r"\toprule",
             r"Phiên bản & " + " & ".join(f"$p={p}$" for p in ps) + r" \\", r"\midrule",
             r"Tuần tự & \multicolumn{%d}{c}{%.1f ms} \\" % (len(ps), t_seq * 1e3),
             r"\midrule"]
    for a in algos:
        s = m[m.algo == a].set_index("threads")
        cells = [f"{s.t_med[p] * 1e3:.1f} ({s.speedup[p]:.2f}$\\times$)" if p in s.index
                 else "--" for p in ps]
        lines.append(LABEL[a] + " & " + " & ".join(cells) + r" \\")
    lines += [r"\bottomrule", r"\end{tabular}"]
    with open(os.path.join(GEN, "table_strong.tex"), "w", encoding="utf-8") as f:
        f.write("\n".join(lines) + "\n")
    print("  -> table_strong.tex")


if __name__ == "__main__":
    print("Tong hop ket qua tu", RAW)
    fig_size()
    m = fig_strong()
    fig_strong("strong_small.csv", "strong_small")
    fig_weak()
    fig_block()
    fig_bind(float(m[m.algo == "seq"].t_med.iloc[0]) if m is not None else None)
    fig_compact()
    tex_strong(m)
    print("Xong.")
