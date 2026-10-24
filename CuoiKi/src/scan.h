/*
 * scan.h - Giao diện chung cho các phiên bản Parallel Prefix Sum (inclusive scan).
 *
 * Tất cả các hàm đều tính INCLUSIVE scan với phép cộng:
 *     out[i] = in[0] + in[1] + ... + in[i]
 * Kiểu phần tử là int64_t để tránh tràn số và để phép cộng có tính kết hợp tuyệt đối
 * (kết quả song song phải trùng khớp từng bit với bản tuần tự).
 */
#ifndef SCAN_H
#define SCAN_H

#include <stddef.h>
#include <stdint.h>

typedef int64_t elem_t;

#define CACHE_LINE 64

/* Vùng nhớ làm việc được cấp phát TRƯỚC khi đo thời gian. */
typedef struct {
    elem_t *tmp;      /* buffer phụ, dung lượng >= next_pow2(n) phần tử  */
    size_t  tmp_cap;
    size_t  block;    /* kích thước khối/luồng/tile cho blocked_opt (số phần tử) */
} scan_ws;

typedef void (*scan_fn)(const elem_t *in, elem_t *out, size_t n, scan_ws *ws);

/* Các phiên bản */
void scan_seq          (const elem_t *in, elem_t *out, size_t n, scan_ws *ws);
void scan_hillis_steele(const elem_t *in, elem_t *out, size_t n, scan_ws *ws);
void scan_blelloch     (const elem_t *in, elem_t *out, size_t n, scan_ws *ws);
void scan_blocked      (const elem_t *in, elem_t *out, size_t n, scan_ws *ws);
void scan_blocked_opt  (const elem_t *in, elem_t *out, size_t n, scan_ws *ws);
void scan_omp          (const elem_t *in, elem_t *out, size_t n, scan_ws *ws);
/* Không phải scan: sao chép song song, dùng làm mốc băng thông bộ nhớ */
void kernel_copy       (const elem_t *in, elem_t *out, size_t n, scan_ws *ws);

typedef struct {
    const char *name;
    scan_fn     fn;
    int         is_scan;   /* 0 với kernel tham chiếu (copy) */
} scan_algo;

extern const scan_algo SCAN_ALGOS[];
extern const int       SCAN_NALGOS;

const scan_algo *scan_find(const char *name);
size_t  next_pow2(size_t n);
scan_ws scan_ws_create(size_t n, size_t block);
void    scan_ws_destroy(scan_ws *ws);
elem_t *alloc_elems(size_t n);   /* cấp phát căn lề 64 byte */

#define DEFAULT_BLOCK ((size_t)1 << 15)   /* 32768 phần tử = 256 KiB / luồng */

#endif /* SCAN_H */
