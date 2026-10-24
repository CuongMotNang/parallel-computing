/*
 * bench.c - Đo hiệu năng các phiên bản scan.
 *
 * Cách dùng:
 *   bench -a <algo[,algo..]|all> -n <log2n[,..]> -t <threads[,..]> [-r reps] [-w warmup]
 *         [-b block] [--header]
 * In ra CSV (stdout):
 *   algo,n,threads,block,rep,time_s
 * Mỗi lần đo: chạy warm-up, sau đó `reps` lần đo độc lập bằng omp_get_wtime().
 * Kết quả mỗi lần đều được kiểm tra với bản tuần tự (sai -> thoát mã lỗi 2).
 */
#include "scan.h"

#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXLIST 64

static int parse_list(const char *s, long *out)
{
    int k = 0;
    char *buf = strdup(s), *tok, *save = NULL;
    for (tok = strtok_r(buf, ",", &save); tok && k < MAXLIST; tok = strtok_r(NULL, ",", &save))
        out[k++] = strtol(tok, NULL, 10);
    free(buf);
    return k;
}

static void usage(void)
{
    fprintf(stderr,
            "bench -a algo[,..]|all -n log2n[,..] -t threads[,..] [-r reps] [-w warmup] "
            "[-b block] [--header]\n  algos:");
    for (int i = 0; i < SCAN_NALGOS; i++) fprintf(stderr, " %s", SCAN_ALGOS[i].name);
    fprintf(stderr, "\n");
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
    const char *algos = "all";
    long ns[MAXLIST], ts[MAXLIST];
    int  nn = 0, nt = 0, reps = 10, warm = 2, header = 0;
    long block = DEFAULT_BLOCK;

    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-a") && i + 1 < argc)      algos = argv[++i];
        else if (!strcmp(argv[i], "-n") && i + 1 < argc) nn = parse_list(argv[++i], ns);
        else if (!strcmp(argv[i], "-t") && i + 1 < argc) nt = parse_list(argv[++i], ts);
        else if (!strcmp(argv[i], "-r") && i + 1 < argc) reps = atoi(argv[++i]);
        else if (!strcmp(argv[i], "-w") && i + 1 < argc) warm = atoi(argv[++i]);
        else if (!strcmp(argv[i], "-b") && i + 1 < argc) block = atol(argv[++i]);
        else if (!strcmp(argv[i], "--header"))           header = 1;
        else usage();
    }
    if (!nn || !nt) usage();

    /* danh sách thuật toán */
    const scan_algo *list[32];
    int na = 0;
    if (!strcmp(algos, "all")) {
        for (int i = 0; i < SCAN_NALGOS; i++) list[na++] = &SCAN_ALGOS[i];
    } else {
        char *buf = strdup(algos), *tok, *save = NULL;
        for (tok = strtok_r(buf, ",", &save); tok; tok = strtok_r(NULL, ",", &save)) {
            const scan_algo *a = scan_find(tok);
            if (!a) { fprintf(stderr, "Khong co thuat toan: %s\n", tok); usage(); }
            list[na++] = a;
        }
        free(buf);
    }

    if (header) printf("algo,n,threads,block,rep,time_s\n");

    for (int ni = 0; ni < nn; ni++) {
        const size_t n = (size_t)1 << ns[ni];
        elem_t *in  = alloc_elems(n);
        elem_t *out = alloc_elems(n);
        elem_t *ref = alloc_elems(n);

        /* first-touch song song với số luồng tối đa */
        omp_set_num_threads(omp_get_num_procs());
        #pragma omp parallel for schedule(static)
        for (size_t i = 0; i < n; i++) {
            uint64_t z = (uint64_t)i * 0x9E3779B97F4A7C15ull;   /* dữ liệu giả ngẫu nhiên, tất định */
            in[i]  = (elem_t)((z >> 40) % 2001) - 1000;
            out[i] = 0;
            ref[i] = 0;
        }
        scan_seq(in, ref, n, NULL);
        scan_ws ws = scan_ws_create(n, (size_t)block);

        for (int ai = 0; ai < na; ai++) {
            const scan_algo *a = list[ai];
            for (int ti = 0; ti < nt; ti++) {
                const int p = (int)ts[ti];
                if (a->fn == scan_seq && p != 1) continue;   /* seq chỉ đo với 1 luồng */
                omp_set_num_threads(p);

                for (int w = 0; w < warm; w++) a->fn(in, out, n, &ws);
                if (a->is_scan && memcmp(out, ref, n * sizeof(elem_t)) != 0) {
                    fprintf(stderr, "SAI KET QUA: %s n=2^%ld p=%d\n", a->name, ns[ni], p);
                    return 2;
                }
                for (int r = 0; r < reps; r++) {
                    const double t0 = omp_get_wtime();
                    a->fn(in, out, n, &ws);
                    const double t1 = omp_get_wtime();
                    printf("%s,%zu,%d,%ld,%d,%.9f\n", a->name, n, p, block, r, t1 - t0);
                }
                fflush(stdout);
            }
        }
        scan_ws_destroy(&ws);
        free(in);
        free(out);
        free(ref);
    }
    return 0;
}
