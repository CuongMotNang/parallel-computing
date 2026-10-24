/*
 * compact.c - Ứng dụng: Stream compaction (lọc các phần tử thỏa điều kiện) bằng scan.
 *
 *   seq   : vòng lặp tuần tự.
 *   par3  : cách "sách giáo khoa", 3 lượt qua bộ nhớ:
 *             flag[i] = pred(in[i]);  pos = scan(flag);  if flag[i]: out[pos[i]-1] = in[i]
 *   fused : hợp nhất theo cấu trúc của scan chia tile:
 *             pha 1 mỗi luồng ĐẾM số phần tử thỏa điều kiện trong khối của mình,
 *             pha 2 tính offset = tổng các khối trước,
 *             pha 3 ghi phần tử thỏa điều kiện vào out[offset...] (đọc lại khối từ L2).
 *           Không cần mảng flag/pos.
 * In CSV: version,n,threads,rep,time_s,kept
 */
#include "scan.h"

#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_THREADS 256

static inline int pred(elem_t x) { return x > 0 && (x % 3 == 0); }

static size_t compact_seq(const elem_t *in, elem_t *out, size_t n)
{
    size_t k = 0;
    for (size_t i = 0; i < n; i++)
        if (pred(in[i])) out[k++] = in[i];
    return k;
}

static size_t compact_par3(const elem_t *in, elem_t *out, size_t n,
                           elem_t *flag, elem_t *pos, scan_ws *ws)
{
    #pragma omp parallel for simd schedule(static)
    for (size_t i = 0; i < n; i++) flag[i] = pred(in[i]);

    scan_blocked_opt(flag, pos, n, ws);            /* inclusive scan */
    const size_t kept = n ? (size_t)pos[n - 1] : 0;

    #pragma omp parallel for schedule(static)
    for (size_t i = 0; i < n; i++)
        if (flag[i]) out[pos[i] - 1] = in[i];      /* exclusive = inclusive - 1 */
    return kept;
}

typedef struct {
    size_t v;
    char   pad[CACHE_LINE - sizeof(size_t)];
} padded_cnt;

static size_t compact_fused(const elem_t *in, elem_t *out, size_t n, size_t B)
{
    static padded_cnt part[2][MAX_THREADS];
    size_t kept = 0;

    #pragma omp parallel
    {
        const int    p     = omp_get_num_threads();
        const int    t     = omp_get_thread_num();
        const size_t tile  = B * (size_t)p;
        size_t       carry = 0;
        int          buf   = 0;

        for (size_t base = 0; base < n; base += tile) {
            const size_t lo = base + (size_t)t * B < n ? base + (size_t)t * B : n;
            const size_t hi = lo + B < n ? lo + B : n;

            size_t c = 0;                                   /* pha 1: đếm */
            #pragma omp simd reduction(+:c)
            for (size_t i = lo; i < hi; i++) c += (size_t)pred(in[i]);
            part[buf][t].v = c;

            #pragma omp barrier

            size_t off = carry, tot = 0;                    /* pha 2: offset */
            for (int j = 0; j < p; j++) {
                const size_t v = part[buf][j].v;
                if (j < t) off += v;
                tot += v;
            }
            carry += tot;

            for (size_t i = lo; i < hi; i++)                /* pha 3: ghi */
                if (pred(in[i])) out[off++] = in[i];
            buf ^= 1;
        }
        #pragma omp single
        kept = carry;
    }
    return kept;
}

static size_t run(int v, const elem_t *in, elem_t *out, size_t n,
                  elem_t *flag, elem_t *pos, scan_ws *ws)
{
    return v == 0 ? compact_par3(in, out, n, flag, pos, ws)
                  : compact_fused(in, out, n, ws->block);
}

int main(int argc, char **argv)
{
    const int log2n = argc > 1 ? atoi(argv[1]) : 26;
    const int reps  = argc > 2 ? atoi(argv[2]) : 10;
    const size_t n  = (size_t)1 << log2n;
    const int threads[] = {1, 2, 4, 6, 8, 10, 12};
    const char *names[] = {"par3", "fused"};

    elem_t *in = alloc_elems(n), *out = alloc_elems(n), *ref = alloc_elems(n);
    elem_t *flag = alloc_elems(n), *pos = alloc_elems(n);
    #pragma omp parallel for schedule(static)
    for (size_t i = 0; i < n; i++) {
        uint64_t z = (uint64_t)i * 0x9E3779B97F4A7C15ull;
        in[i] = (elem_t)((z >> 40) % 2001) - 1000;
        out[i] = ref[i] = flag[i] = pos[i] = 0;
    }
    scan_ws ws = scan_ws_create(n, DEFAULT_BLOCK);

    printf("version,n,threads,rep,time_s,kept\n");
    const size_t kref = compact_seq(in, ref, n);
    for (int r = 0; r < reps; r++) {
        double t0 = omp_get_wtime();
        size_t k = compact_seq(in, ref, n);
        double t1 = omp_get_wtime();
        printf("seq,%zu,1,%d,%.9f,%zu\n", n, r, t1 - t0, k);
    }
    for (size_t ti = 0; ti < sizeof(threads) / sizeof(threads[0]); ti++) {
        omp_set_num_threads(threads[ti]);
        for (int v = 0; v < 2; v++) {
            memset(out, 0, n * sizeof(elem_t));
            size_t k = run(v, in, out, n, flag, pos, &ws);
            if (k != kref || memcmp(out, ref, k * sizeof(elem_t)) != 0) {
                fprintf(stderr, "SAI KET QUA compaction %s p=%d\n", names[v], threads[ti]);
                return 2;
            }
            for (int r = 0; r < reps; r++) {
                double t0 = omp_get_wtime();
                k = run(v, in, out, n, flag, pos, &ws);
                double t1 = omp_get_wtime();
                printf("%s,%zu,%d,%d,%.9f,%zu\n", names[v], n, threads[ti], r, t1 - t0, k);
            }
        }
    }
    scan_ws_destroy(&ws);
    free(in); free(out); free(ref); free(flag); free(pos);
    return 0;
}
