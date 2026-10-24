/*
 * scan.c - Cài đặt các phiên bản Parallel Prefix Sum bằng OpenMP.
 *
 *   seq            : tuần tự, O(n)                                   (baseline)
 *   hillis_steele  : Hillis & Steele 1986, double buffer, O(n log n)  (không work-efficient)
 *   blelloch       : Blelloch 1990, up-sweep/down-sweep tại chỗ, O(n)
 *   blocked        : chia khối 3 pha (reduce - scan tổng khối - scan cục bộ), O(n)
 *   blocked_opt    : blocked + chia tile vừa cache L2 + chống false sharing + SIMD
 *   omp_scan       : chỉ thị chuẩn OpenMP 5.0  reduction(inscan, +)
 */
#include "scan.h"

#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ------------------------------------------------------------------ */
/* Tiện ích                                                            */
/* ------------------------------------------------------------------ */
size_t next_pow2(size_t n)
{
    size_t m = 1;
    while (m < n) m <<= 1;
    return m;
}

elem_t *alloc_elems(size_t n)
{
    size_t bytes = (n ? n : 1) * sizeof(elem_t);
    bytes = (bytes + CACHE_LINE - 1) / CACHE_LINE * CACHE_LINE;  /* aligned_alloc yêu cầu bội số */
    elem_t *p = aligned_alloc(CACHE_LINE, bytes);
    if (!p) {
        fprintf(stderr, "Khong du bo nho (%zu bytes)\n", bytes);
        exit(EXIT_FAILURE);
    }
    return p;
}

scan_ws scan_ws_create(size_t n, size_t block)
{
    scan_ws ws;
    ws.tmp_cap = next_pow2(n ? n : 1);
    ws.tmp     = alloc_elems(ws.tmp_cap);
    ws.block   = block ? block : DEFAULT_BLOCK;
    /* first-touch song song: ánh xạ trang trước khi đo */
    #pragma omp parallel for schedule(static)
    for (size_t i = 0; i < ws.tmp_cap; i++) ws.tmp[i] = 0;
    return ws;
}

void scan_ws_destroy(scan_ws *ws)
{
    free(ws->tmp);
    ws->tmp = NULL;
    ws->tmp_cap = 0;
}

/* ------------------------------------------------------------------ */
/* 1. Tuần tự                                                          */
/* ------------------------------------------------------------------ */
void scan_seq(const elem_t *in, elem_t *out, size_t n, scan_ws *ws)
{
    (void)ws;
    elem_t s = 0;
    for (size_t i = 0; i < n; i++) {
        s += in[i];
        out[i] = s;
    }
}

/* ------------------------------------------------------------------ */
/* 2. Hillis-Steele: log2(n) bước, mỗi bước cộng phần tử cách off       */
/*    Cần double buffer vì các luồng không chạy đồng bộ (lockstep).    */
/* ------------------------------------------------------------------ */
void scan_hillis_steele(const elem_t *in, elem_t *out, size_t n, scan_ws *ws)
{
    if (n == 0) return;
    elem_t *a = out;      /* buffer đọc  */
    elem_t *b = ws->tmp;  /* buffer ghi  */

    #pragma omp parallel
    {
        #pragma omp for schedule(static)
        for (size_t i = 0; i < n; i++) a[i] = in[i];

        for (size_t off = 1; off < n; off <<= 1) {
            /* Hai vòng lặp tách riêng để tránh rẽ nhánh -> trình biên dịch vector hóa được */
            #pragma omp for schedule(static) nowait
            for (size_t i = 0; i < off; i++) b[i] = a[i];
            #pragma omp for schedule(static)
            for (size_t i = off; i < n; i++) b[i] = a[i - off] + a[i];
            /* rào chắn ngầm định ở cuối vòng for ở trên */
            #pragma omp single
            { elem_t *t = a; a = b; b = t; }   /* hoán đổi buffer; single có rào chắn ngầm định */
        }

        if (a != out) {
            #pragma omp for schedule(static)
            for (size_t i = 0; i < n; i++) out[i] = a[i];
        }
    }
}

/* ------------------------------------------------------------------ */
/* 3. Blelloch: up-sweep (reduce) + down-sweep trên cây nhị phân ẩn.   */
/*    Làm tại chỗ trên buffer kích thước m = next_pow2(n) (pad bằng 0). */
/*    Kết quả tự nhiên là exclusive scan; cộng in[i] để ra inclusive.  */
/* ------------------------------------------------------------------ */
void scan_blelloch(const elem_t *in, elem_t *out, size_t n, scan_ws *ws)
{
    if (n == 0) return;
    const size_t m = next_pow2(n);
    elem_t *x = ws->tmp;

    #pragma omp parallel
    {
        #pragma omp for schedule(static)
        for (size_t i = 0; i < m; i++) x[i] = (i < n) ? in[i] : 0;

        /* Up-sweep: stride = 2^(d+1) */
        for (size_t stride = 2; stride <= m; stride <<= 1) {
            const size_t half = stride >> 1;
            #pragma omp for schedule(static)
            for (size_t k = stride - 1; k < m; k += stride)
                x[k] += x[k - half];
        }

        #pragma omp single
        x[m - 1] = 0;                       /* gán phần tử đơn vị cho gốc */

        /* Down-sweep */
        for (size_t stride = m; stride >= 2; stride >>= 1) {
            const size_t half = stride >> 1;
            #pragma omp for schedule(static)
            for (size_t k = stride - 1; k < m; k += stride) {
                elem_t t    = x[k - half];  /* giá trị con trái từ up-sweep   */
                x[k - half] = x[k];         /* con trái  <- giá trị của cha    */
                x[k]       += t;            /* con phải  <- cha + con trái cũ  */
            }
        }

        /* exclusive -> inclusive */
        #pragma omp for schedule(static)
        for (size_t i = 0; i < n; i++) out[i] = x[i] + in[i];
    }
}

/* ------------------------------------------------------------------ */
/* 4. Blocked (reduce-then-scan), mỗi luồng một khối liên tiếp n/p:    */
/*    Pha 1: mỗi luồng tính tổng khối của mình                         */
/*    Pha 2: một luồng scan mảng p tổng khối -> offset                  */
/*    Pha 3: mỗi luồng scan tuần tự khối của mình, bắt đầu từ offset    */
/* ------------------------------------------------------------------ */
#define MAX_THREADS 256

void scan_blocked(const elem_t *in, elem_t *out, size_t n, scan_ws *ws)
{
    (void)ws;
    if (n == 0) return;
    elem_t sums[MAX_THREADS + 1];

    #pragma omp parallel
    {
        const int    p   = omp_get_num_threads();
        const int    t   = omp_get_thread_num();
        const size_t len = (n + p - 1) / p;
        const size_t lo  = (size_t)t * len < n ? (size_t)t * len : n;
        const size_t hi  = lo + len < n ? lo + len : n;

        /* Pha 1 */
        elem_t s = 0;
        for (size_t i = lo; i < hi; i++) s += in[i];
        sums[t + 1] = s;
        #pragma omp barrier

        /* Pha 2 */
        #pragma omp single
        {
            sums[0] = 0;
            for (int i = 1; i <= p; i++) sums[i] += sums[i - 1];
        }   /* rào chắn ngầm định */

        /* Pha 3 */
        s = sums[t];
        for (size_t i = lo; i < hi; i++) {
            s += in[i];
            out[i] = s;
        }
    }
}

/* ------------------------------------------------------------------ */
/* 5. Blocked tối ưu:                                                   */
/*  - Chia mảng thành các TILE gồm p khối, mỗi khối B phần tử (vừa L2).  */
/*    Pha 1 đọc khối từ RAM vào cache, pha 3 đọc lại NGAY từ cache      */
/*    => mảng vào chỉ đọc từ RAM 1 lần (16 B/phần tử thay vì 24 B).     */
/*  - Mảng tổng khối được pad 64 byte (chống false sharing),            */
/*    double-buffer theo tile => chỉ 1 barrier mỗi tile.               */
/*  - Mỗi luồng tự tính offset của mình (p nhỏ) => không cần single.   */
/*  - Pha 1 dùng omp simd reduction (vector hóa AVX2).                 */
/* ------------------------------------------------------------------ */
typedef struct {
    elem_t v;
    char   pad[CACHE_LINE - sizeof(elem_t)];
} padded_elem;

void scan_blocked_opt(const elem_t *in, elem_t *out, size_t n, scan_ws *ws)
{
    if (n == 0) return;
    const size_t B = ws->block;
    static padded_elem part[2][MAX_THREADS];

    #pragma omp parallel
    {
        const int    p     = omp_get_num_threads();
        const int    t     = omp_get_thread_num();
        const size_t tile  = B * (size_t)p;
        elem_t       carry = 0;            /* tổng mọi tile trước (mỗi luồng giữ 1 bản) */
        int          buf   = 0;

        for (size_t base = 0; base < n; base += tile) {
            const size_t lo = base + (size_t)t * B < n ? base + (size_t)t * B : n;
            const size_t hi = lo + B < n ? lo + B : n;

            /* Pha 1: tổng khối (vector hóa) */
            elem_t s = 0;
            #pragma omp simd reduction(+:s)
            for (size_t i = lo; i < hi; i++) s += in[i];
            part[buf][t].v = s;

            #pragma omp barrier

            /* Pha 2: offset = carry + tổng các khối đứng trước; cập nhật carry */
            elem_t off = carry, tot = 0;
            for (int j = 0; j < p; j++) {
                const elem_t v = part[buf][j].v;
                if (j < t) off += v;
                tot += v;
            }
            carry += tot;

            /* Pha 3: scan cục bộ, dữ liệu đang nằm trong L2 */
            s = off;
            for (size_t i = lo; i < hi; i++) {
                s += in[i];
                out[i] = s;
            }
            buf ^= 1;   /* tile kế tiếp ghi sang buffer kia: không cần barrier thứ hai */
        }
    }
}

/* ------------------------------------------------------------------ */
/* 6. Chỉ thị chuẩn OpenMP 5.0: reduction(inscan, +) + #pragma omp scan */
/* ------------------------------------------------------------------ */
void scan_omp(const elem_t *in, elem_t *out, size_t n, scan_ws *ws)
{
    (void)ws;
    if (n == 0) return;   /* libgomp (GCC 15) bị lỗi bộ nhớ khi vòng inscan rỗng và p > 1 */
    elem_t s = 0;
    /* Lưu ý: chuẩn OpenMP không cho phép mệnh đề schedule đi cùng reduction(inscan).
     * Không dùng "for simd": GCC cấp phát mảng tạm cỡ n trên stack -> tràn stack với n lớn. */
    #pragma omp parallel for reduction(inscan, +:s)
    for (size_t i = 0; i < n; i++) {
        s += in[i];
        #pragma omp scan inclusive(s)
        out[i] = s;
    }
}

/* ------------------------------------------------------------------ */
/* Kernel tham chiếu: sao chép song song (đọc n + ghi n)                */
/* Dùng để đo băng thông bộ nhớ thực tế đạt được (STREAM-copy).        */
/* ------------------------------------------------------------------ */
void kernel_copy(const elem_t *in, elem_t *out, size_t n, scan_ws *ws)
{
    (void)ws;
    #pragma omp parallel for simd schedule(static)
    for (size_t i = 0; i < n; i++) out[i] = in[i];
}

/* ------------------------------------------------------------------ */
const scan_algo SCAN_ALGOS[] = {
    {"seq",           scan_seq,           1},
    {"hillis_steele", scan_hillis_steele, 1},
    {"blelloch",      scan_blelloch,      1},
    {"blocked",       scan_blocked,       1},
    {"blocked_opt",   scan_blocked_opt,   1},
    {"omp_scan",      scan_omp,           1},
    {"copy",          kernel_copy,        0},
};
const int SCAN_NALGOS = (int)(sizeof(SCAN_ALGOS) / sizeof(SCAN_ALGOS[0]));

const scan_algo *scan_find(const char *name)
{
    for (int i = 0; i < SCAN_NALGOS; i++)
        if (strcmp(SCAN_ALGOS[i].name, name) == 0) return &SCAN_ALGOS[i];
    return NULL;
}
