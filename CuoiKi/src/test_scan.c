/*
 * test_scan.c - Kiểm thử tính đúng: so sánh mọi phiên bản với scan_seq
 * trên nhiều kích thước (kể cả 0, 1, không phải lũy thừa 2) và nhiều số luồng.
 */
#include "scan.h"

#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

static int check(const scan_algo *a, const elem_t *in, const elem_t *ref, size_t n,
                 int threads, size_t block)
{
    elem_t *out = alloc_elems(n);
    for (size_t i = 0; i < n; i++) out[i] = -12345;
    scan_ws ws = scan_ws_create(n, block);
    omp_set_num_threads(threads);
    a->fn(in, out, n, &ws);
    int ok = 1;
    for (size_t i = 0; i < n; i++)
        if (out[i] != ref[i]) {
            fprintf(stderr, "  FAIL %-14s n=%zu p=%d block=%zu tai i=%zu: %lld != %lld\n",
                    a->name, n, threads, block, i, (long long)out[i], (long long)ref[i]);
            ok = 0;
            break;
        }
    scan_ws_destroy(&ws);
    free(out);
    return ok;
}

int main(void)
{
    const size_t sizes[]   = {0, 1, 2, 3, 5, 8, 13, 64, 100, 1000, 4096, 4099,
                              65537, 1000003, (1u << 20) + 7};
    const int    threads[] = {1, 2, 3, 4, 7, 8, 12};
    const size_t blocks[]  = {1, 3, 1024, DEFAULT_BLOCK};
    int total = 0, pass = 0;

    /* Ví dụ trong Blelloch (1990) */
    {
        elem_t in[8] = {3, 1, 7, 0, 4, 1, 6, 3}, out[8];
        scan_ws ws = scan_ws_create(8, 0);
        scan_blelloch(in, out, 8, &ws);
        printf("Vi du Blelloch: ");
        for (int i = 0; i < 8; i++) printf("%lld ", (long long)out[i]);
        printf(" (mong doi: 3 4 11 11 15 16 22 25)\n");
        scan_ws_destroy(&ws);
    }

    srand(2026);
    for (size_t si = 0; si < sizeof(sizes) / sizeof(sizes[0]); si++) {
        const size_t n = sizes[si];
        elem_t *in  = alloc_elems(n);
        elem_t *ref = alloc_elems(n);
        for (size_t i = 0; i < n; i++) in[i] = (rand() % 2001) - 1000;  /* có cả số âm */
        scan_seq(in, ref, n, NULL);

        for (int a = 0; a < SCAN_NALGOS; a++) {
            if (!SCAN_ALGOS[a].is_scan) continue;
            for (size_t ti = 0; ti < sizeof(threads) / sizeof(threads[0]); ti++)
                for (size_t bi = 0; bi < sizeof(blocks) / sizeof(blocks[0]); bi++) {
                    /* block chỉ có ý nghĩa với blocked_opt */
                    if (bi > 0 && SCAN_ALGOS[a].fn != scan_blocked_opt) continue;
                    total++;
                    pass += check(&SCAN_ALGOS[a], in, ref, n, threads[ti], blocks[bi]);
                }
        }
        free(in);
        free(ref);
    }
    printf("Ket qua: %d/%d test dat\n", pass, total);
    return pass == total ? 0 : 1;
}
