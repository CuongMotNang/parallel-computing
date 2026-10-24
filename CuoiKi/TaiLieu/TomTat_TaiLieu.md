# Tóm tắt tài liệu tham khảo – Parallel Prefix Sum (Scan)

> Ghi chú do nhóm tự tóm tắt để làm cơ sở cho phần cài đặt OpenMP. Xem bản gốc ở link bên dưới.

| # | Tài liệu | Link |
|---|---|---|
| [1] | G. E. Blelloch, *Prefix Sums and Their Applications*, CMU-CS-90-190, 1990 | https://www.cs.cmu.edu/~scandal/papers/CMU-CS-90-190.html (PDF: http://www.cs.cmu.edu/~guyb/papers/Ble93.pdf) |
| [2] | M. Harris, S. Sengupta, J. D. Owens, *Parallel Prefix Sum (Scan) with CUDA*, GPU Gems 3, Ch. 39, 2007 | https://developer.nvidia.com/gpugems/GPUGems3/gpugems3_ch39.html |

---

## 1. Định nghĩa

Cho phép toán **kết hợp** ⊕ (ví dụ phép cộng) có phần tử đơn vị I và mảng `[a0, a1, ..., a(n-1)]`:

| Loại | Kết quả | Ví dụ với `[3 1 7 0 4 1 6 3]` |
|---|---|---|
| **Inclusive scan** (scan, [1]) | `[a0, a0⊕a1, ..., a0⊕...⊕a(n-1)]` | `[3 4 11 11 15 16 22 25]` |
| **Exclusive scan** (prescan [1], "scan" [2]) | `[I, a0, a0⊕a1, ..., a0⊕...⊕a(n-2)]` | `[0 3 4 11 11 15 16 22]` |

- Đổi qua lại: exclusive bằng inclusive dịch phải 1 vị trí rồi chèn I vào đầu. Inclusive bằng exclusive dịch trái 1 vị trí rồi thêm tổng cuối vào cuối mảng.
- **Bản tuần tự:** một vòng lặp gồm n phép cộng, O(n). Đây là mốc để đánh giá *work-efficiency*.
- Lưu ý: chỉ cần ⊕ có **tính kết hợp**, không cần giao hoán. Với số thực (float), phép cộng không hoàn toàn kết hợp, nên kết quả song song có thể sai lệch nhỏ so với bản tuần tự. Khi kiểm thử nên dùng số nguyên hoặc so sánh có sai số cho phép.

## 2. Các thuật toán

### 2.1 Naive scan – Hillis & Steele (1986) [2, mục 39.2.1]
```
for d = 1 .. log2 n:
    for all k in parallel:
        if k >= 2^(d-1):  x_out[k] = x_in[k - 2^(d-1)] + x_in[k]
        else:             x_out[k] = x_in[k]
    swap(x_in, x_out)          // double buffering
```
- **Số bước:** log2 n (step-efficient).
- **Tổng số phép cộng:** O(n log n), nên **không work-efficient**.
- Cần **2 bộ đệm** (double-buffer), vì nếu làm tại chỗ thì các luồng sẽ đọc phải giá trị đã bị luồng khác ghi đè.

### 2.2 Work-efficient scan – Blelloch (1990) [1, mục 1.2; 2, mục 39.2.2]
Coi mảng như một **cây nhị phân cân bằng** và làm tại chỗ. Yêu cầu n là lũy thừa của 2; nếu không thì pad thêm phần tử đơn vị.

**Up-sweep (reduce):** đi từ lá lên gốc
```
for d = 0 .. log2 n - 1:
    for all k = 0 .. n-1 step 2^(d+1) in parallel:
        x[k + 2^(d+1) - 1] += x[k + 2^d - 1]
```
**Down-sweep:** đi từ gốc xuống lá
```
x[n-1] = 0
for d = log2 n - 1 downto 0:
    for all k = 0 .. n-1 step 2^(d+1) in parallel:
        t = x[k + 2^d - 1]
        x[k + 2^d - 1]      = x[k + 2^(d+1) - 1]
        x[k + 2^(d+1) - 1] += t
```
Ví dụ [1, Hình 1.4]:
```
input        [3 1 7  0 4  1 6  3]
up d=0       [3 4 7  7 4  5 6  9]
up d=1       [3 4 7 11 4  5 6 14]
up d=2       [3 4 7 11 4  5 6 25]
clear        [3 4 7 11 4  5 6  0]
down d=2     [3 4 7  0 4  5 6 11]
down d=1     [3 0 7  4 4 11 6 16]
down d=0     [0 3 4 11 11 15 16 22]   ← exclusive scan
```
- **Số bước:** 2·log2 n. **Công việc:** 2(n−1) phép cộng và n−1 phép hoán đổi, tức O(n), nên **work-efficient**.
- Tính đúng [1, Định lý 1.1]: sau down-sweep, mỗi nút chứa tổng của mọi lá đứng trước nó theo thứ tự duyệt tiền thứ tự (preorder).
- Nhược điểm trên CPU: ở các tầng trên cây chỉ còn rất ít phần tử nên thiếu việc cho các luồng. Truy cập bộ nhớ có bước nhảy 2^d nên **kém thân thiện với cache**. Mỗi tầng cần một lần đồng bộ (barrier), tổng cộng 2·log2 n barrier.

### 2.3 Khi n > p: chia khối (blocked / chunked scan) [1, Hình 1.6; 2, mục 39.2.4]
Đây là cách **thực tế nhất trên CPU đa nhân**, vì số luồng p (khoảng 4–16) nhỏ hơn rất nhiều so với n (hàng triệu):
1. Chia mảng thành p khối liên tiếp. Mỗi luồng **tính tổng khối của mình** theo kiểu tuần tự.
2. Thực hiện **exclusive scan trên p tổng khối**. Vì p nhỏ nên có thể làm tuần tự hoặc dùng cây.
3. Mỗi luồng **scan tuần tự khối của mình**, bắt đầu từ offset vừa tính ở bước 2.

```
[4 7 1 | 0 5 2 | 6 4 8 | 1 9 5]     tổng khối = [12 7 18 15]
prescan tổng khối = [0 12 19 37]
→ [0 4 11 | 12 12 17 | 19 25 29 | 37 38 47]
```
- Thời gian theo [1]: T(n,p) = 2(⌈n/p⌉ + ⌈lg p⌉) = O(n/p + lg p). Speedup đạt tối ưu khi n/p ≥ lg p.
- Đọc mảng **2 lần** nên với n lớn thì giới hạn bởi **băng thông bộ nhớ** (memory-bound). Đây là điểm cần phân tích khi đo hiệu năng.
- GPU Gems [2] dùng đúng ý này: scan từng block, ghi tổng block vào mảng `SUMS`, scan `SUMS` thành `INCR`, rồi cộng đều `INCR[i]` vào block i (uniform add).

### 2.4 Kỹ thuật tối ưu trong [2] và cách chuyển sang CPU/OpenMP
| GPU (CUDA) | Tương đương trên CPU |
|---|---|
| Bank conflict ở shared memory: thêm padding `CONFLICT_FREE_OFFSET` | **False sharing** ở cache line 64 byte: pad mảng tổng khối hoặc dùng biến cục bộ |
| Mỗi luồng xử lý 8 phần tử thay vì 2 (nhanh hơn hơn 2 lần, theo định lý Brent) | Mỗi luồng xử lý **một khối lớn**, làm tuần tự trong khối (chính là blocked scan) |
| Unroll vòng lặp | `-O2/-O3`, để trình biên dịch vector hóa (SIMD) |
| Kết quả: nhanh hơn tới khoảng 20 lần so với scan tuần tự trên CPU | Trên CPU kỳ vọng speedup bị **giới hạn bởi băng thông RAM** |

## 3. Ứng dụng (cho phần mở đầu báo cáo)
Theo [1] và [2]:
- radix sort, quicksort
- **stream compaction** (xóa phần tử được đánh dấu)
- line-of-sight (scan với phép max)
- đánh giá đa thức, cộng số nhiều chữ số
- giải hệ truy hồi và hệ ba đường chéo
- phân tích từ vựng, histogram
- summed-area table (xử lý ảnh)
- cấp phát bộ nhớ hoặc bộ xử lý động

## 4. Gợi ý các phiên bản cần cài đặt và so sánh (chờ yêu cầu chi tiết)
| Phiên bản | Ý tưởng | Work | Ghi chú |
|---|---|---|---|
| `seq` | vòng lặp tuần tự | O(n) | baseline |
| `hillis_steele` | 2.1 + `#pragma omp parallel for`, double buffer | O(n log n) | minh họa không work-efficient |
| `blelloch` | 2.2 up/down-sweep tại chỗ | O(n) | nhiều barrier, kém cache |
| `blocked` | 2.3, 3 pha, mỗi luồng một khối | O(n) | kỳ vọng nhanh nhất |
| (tùy chọn) `omp scan` | `#pragma omp for reduction(inscan, +:x)` (OpenMP 5.0) | — | so sánh với chỉ thị chuẩn |

Chỉ số đánh giá: thời gian chạy (`omp_get_wtime`, lấy trung vị nhiều lần đo), speedup S = T_seq / T_p, efficiency E = S/p, thông lượng (phần tử/giây hoặc GB/s). Thay đổi n (2^20 → 2^28) và số luồng p (1, 2, 4, 8, 12).
