#!/usr/bin/env bash
# Chạy toàn bộ thực nghiệm, ghi CSV vào results/raw/.
# Dùng:  bash scripts/run_all.sh            (đầy đủ, ~15-25 phút)
#        QUICK=1 bash scripts/run_all.sh    (chạy nhanh để thử)
# Khuyến nghị: cắm sạc, chế độ High/Best performance, tắt ứng dụng nền.
set -euo pipefail
cd "$(dirname "$0")/.."

make -s all
./build/test_scan | tail -1

OUT=results/raw
mkdir -p "$OUT"
B=./build/bench
R=${REPS:-10}
THREADS=1,2,4,6,8,10,12
SCANS=seq,blelloch,blocked,blocked_opt,omp_scan,copy

if [[ "${QUICK:-0}" == 1 ]]; then R=3; fi

export OMP_PROC_BIND=${OMP_PROC_BIND:-false}

echo "== Thong tin may =="
{
    echo "date: $(date -Is)"
    echo "kernel: $(uname -r)"
    lscpu | grep -E "Model name|^CPU\(s\)|Thread|Core|L1d|L2|L3"
    free -h | head -2
    gcc --version | head -1
    echo "CFLAGS: -O3 -march=native -std=gnu11 -fopenmp"
} | tee "$OUT/machine.txt"

echo "== 1. Theo kich thuoc n (p=12) =="
$B -a $SCANS -n 16,17,18,19,20,21,22,23,24,25,26,27 -t 1,12 -r $R --header > "$OUT/size.csv"
$B -a hillis_steele -n 16,17,18,19,20,21,22,23,24,25,26 -t 1,12 -r 3 >> "$OUT/size.csv"

echo "== 2. Strong scaling (n=2^26) =="
$B -a $SCANS -n 26 -t $THREADS -r $R --header > "$OUT/strong.csv"
$B -a hillis_steele -n 26 -t $THREADS -r 3 >> "$OUT/strong.csv"
echo "   strong scaling n=2^20 (vua cache L3? khong: 8 MiB)"
$B -a $SCANS,hillis_steele -n 20 -t $THREADS -r $((R*3)) --header > "$OUT/strong_small.csv"

echo "== 3. Weak scaling (2^23 phan tu / luong) =="
{
    echo "algo,n,threads,block,rep,time_s"
    for k in 0 1 2 3; do
        p=$((1 << k)); n=$((23 + k))
        $B -a blocked,blocked_opt,omp_scan,blelloch,copy -n $n -t $p -r $R
    done
} > "$OUT/weak.csv"

echo "== 4. Kich thuoc khoi cua blocked_opt (n=2^26, p=12) =="
{
    echo "algo,n,threads,block,rep,time_s"
    for b in 512 1024 2048 4096 8192 16384 32768 65536 131072 262144 524288 1048576 4194304; do
        $B -a blocked_opt -n 26 -t 12 -b $b -r $R
    done
} > "$OUT/block.csv"

echo "== 5. Chinh sach gan luong (OMP_PROC_BIND) =="
{
    echo "bind,algo,n,threads,block,rep,time_s"
    for bind in false close spread; do
        OMP_PROC_BIND=$bind OMP_PLACES=threads $B -a blocked_opt,blocked -n 26 -t $THREADS -r $R \
            | sed "s/^/$bind,/"
    done
} > "$OUT/bind.csv"

echo "== 6. Ung dung: stream compaction (n=2^26) =="
./build/compact 26 $R > "$OUT/compact.csv"

echo "Xong. Ket qua trong $OUT/"
