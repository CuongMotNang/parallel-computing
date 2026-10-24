# Kế hoạch cuối kỳ – Parallel Prefix Sum Using OpenMP: Implementation and Performance Evaluation

| Thành viên | MSSV |
|---|---|
| Bùi Xuân Chung | 23001838 |
| Lê Văn Cường | 23001842 |
| Đào Ngọc Cường | 23001841 |

- Trường Đại học Khoa học Tự nhiên – Đại học Quốc gia Hà Nội
- GVHD: Nguyễn Hải Vinh, Mạc Đăng Trường
- Yêu cầu: **thuyết trình 20 phút**, **báo cáo LaTeX 35–40 trang nội dung chính** (không tính bìa, lời mở đầu, mục lục, tài liệu tham khảo, phụ lục)

---

## A. Bố cục báo cáo (LaTeX)

**Phần đầu (không tính trang):** bìa, lời cảm ơn, tóm tắt, bảng phân công, mục lục, danh mục hình và bảng, danh mục từ viết tắt.

| Chương | Nội dung | Trang | Phụ trách |
|---|---|---|---|
| **1. Giới thiệu** | 1.1 Bối cảnh tính toán song song và vai trò của scan · 1.2 Phát biểu bài toán · 1.3 Mục tiêu và phạm vi · 1.4 Đóng góp của nhóm · 1.5 Cấu trúc báo cáo | 4 | Chung |
| **2. Cơ sở lý thuyết** | 2.1 Prefix sum: inclusive/exclusive, phép toán kết hợp, ví dụ · 2.2 Mô hình PRAM, work/span, work-efficiency, định lý Brent · 2.3 Amdahl, Gustafson, giới hạn băng thông bộ nhớ (roofline) · 2.4 OpenMP: fork–join, `parallel for`, schedule, barrier, reduction, `scan` (OpenMP 5.0) · 2.5 Kiến trúc CPU đa nhân: cache, false sharing, CPU lai P-core/E-core · 2.6 Ứng dụng của scan | 8 | Chung |
| **3. Thuật toán song song cho prefix sum** | 3.1 Thuật toán tuần tự và phụ thuộc dữ liệu · 3.2 Hillis–Steele (minh họa, phân tích O(n log n)) · 3.3 Blelloch up-sweep/down-sweep (minh họa, chứng minh tính đúng, phân tích O(n)) · 3.4 Scan chia khối 3 pha (reduce-then-scan và scan-then-propagate) · 3.5 Chỉ thị `omp scan` · 3.6 Bảng so sánh lý thuyết: work, span, số barrier, lưu lượng bộ nhớ | 8 | L.V.Cường |
| **4. Cài đặt bằng OpenMP** | 4.1 Kiến trúc mã nguồn và giao diện chung · 4.2 Cài đặt từng phiên bản (đoạn code chính) · 4.3 Tối ưu: chống false sharing, first-touch, SIMD, xử lý n không phải lũy thừa của 2 · 4.4 Kiểm thử tính đúng (so với bản tuần tự, các trường hợp biên) · 4.5 Công cụ đo và quy trình benchmark | 6 | L.V.Cường |
| **5. Thực nghiệm và đánh giá** | 5.1 Môi trường (phần cứng, trình biên dịch, cờ biên dịch) · 5.2 Phương pháp đo (warm-up, lặp lại, lấy trung vị, `OMP_PROC_BIND`) · 5.3 Thời gian theo kích thước n · 5.4 Strong scaling: speedup, efficiency · 5.5 Weak scaling · 5.6 Thông lượng GB/s so với băng thông bộ nhớ · 5.7 Ảnh hưởng của kích thước khối, ghim luồng, P-core/E-core · 5.8 Ứng dụng minh họa: stream compaction · 5.9 Thảo luận | 11 | Đ.N.Cường |
| **6. Kết luận và hướng phát triển** | Kết quả đạt được, hạn chế, hướng mở rộng (GPU/CUDA, MPI, scan phân đoạn) | 2 | Đ.N.Cường |
| | **Tổng nội dung chính** | **~39** | |

**Phần cuối (không tính trang):** tài liệu tham khảo (BibTeX), phụ lục A (mã nguồn chính), phụ lục B (bảng số liệu thô).

**Định dạng:** A4, font Times 13pt, giãn dòng 1.3, lề trái 3 cm / phải 2 cm / trên và dưới 2,5 cm. Biên dịch bằng XeLaTeX qua MiKTeX trên máy. Hình thuật toán vẽ bằng TikZ, biểu đồ bằng pgfplots hoặc matplotlib, mã giả dùng `algorithm2e`, code dùng `listings`.

## B. Bố cục slide (Beamer, 20 phút, khoảng 22 slide)

| # | Slide | Thời gian | Người trình bày |
|---|---|---|---|
| 1 | Bìa (trường, tên đề tài, nhóm và MSSV, GVHD) | 0:20 | Chung |
| 2 | Nội dung trình bày | 0:20 | |
| 3 | Đặt vấn đề: scan là gì, vì sao "có vẻ tuần tự" | 1:00 | |
| 4 | Ứng dụng của scan | 0:40 | |
| 5 | Mục tiêu đề tài | 0:30 | |
| 6 | Work, span, work-efficiency, định lý Brent | 1:00 | |
| 7 | OpenMP: mô hình fork–join, các chỉ thị sẽ dùng | 1:00 | **≈ 6:30** |
| 8 | Thuật toán tuần tự và phụ thuộc dữ liệu | 0:30 | L.V.Cường |
| 9 | Hillis–Steele: hình minh họa và độ phức tạp | 1:15 | |
| 10 | Blelloch: up-sweep (hình) | 1:00 | |
| 11 | Blelloch: down-sweep (hình) | 1:00 | |
| 12 | Scan chia khối 3 pha (hình) | 1:15 | |
| 13 | Bảng so sánh lý thuyết các thuật toán | 0:45 | |
| 14 | Cài đặt OpenMP: đoạn code chính của bản chia khối | 1:00 | |
| 15 | Tối ưu và kiểm thử | 0:45 | **≈ 7:30** |
| 16 | Môi trường và phương pháp đo | 0:40 | Đ.N.Cường |
| 17 | Thời gian chạy theo n | 0:50 | |
| 18 | Speedup và efficiency theo số luồng | 1:00 | |
| 19 | Thông lượng so với băng thông bộ nhớ (vì sao speedup bão hòa) | 1:00 | |
| 20 | Ảnh hưởng của P-core/E-core, ghim luồng, kích thước khối | 0:45 | |
| 21 | Kết luận và hướng phát triển | 0:45 | |
| 22 | Cảm ơn và hỏi đáp | 0:10 | **≈ 6:00** |

Slide phụ để dùng khi bị hỏi: chứng minh tính đúng của Blelloch, `omp scan`, bảng số liệu chi tiết, demo chạy trực tiếp.

Kèm theo: **tài liệu cho người thuyết trình** (lời nói và câu hỏi dự phòng), giống phần giữa kỳ.

---

## C. Các phiên bản cài đặt

| Phiên bản | Mô tả | Mục đích |
|---|---|---|
| `seq` | Vòng lặp tuần tự | Baseline |
| `hillis_steele` | Double buffer, `parallel for` mỗi bước | Minh họa thuật toán không work-efficient |
| `blelloch` | Up-sweep/down-sweep tại chỗ, pad lên lũy thừa của 2 | Bản sát với lý thuyết PRAM |
| `blocked` | 3 pha: tổng khối, scan tổng khối, scan cục bộ | Bản thực tế trên CPU |
| `blocked_opt` | `blocked` + chống false sharing + SIMD + first-touch | Bản tối ưu nhất |
| `omp_scan` | `reduction(inscan,+)` theo OpenMP 5.0 | So sánh với chỉ thị chuẩn |

Ứng dụng minh họa: **stream compaction** (lọc phần tử thỏa điều kiện) dùng scan.

**Thực nghiệm dự kiến:**
- n = 2^16 … 2^28, kiểu `int64` hoặc `double`
- p = 1, 2, 4, 6, 8, 10, 12
- mỗi cấu hình chạy 10 lần, lấy trung vị
- xuất CSV, script Python vẽ biểu đồ

---

## D. Các việc cần làm

| # | Việc | Ai làm |
|---|---|---|
| 1 | Cài đặt 6 phiên bản, kiểm thử tính đúng, script benchmark và vẽ biểu đồ | Claude |
| 2 | Khung LaTeX báo cáo (bìa HUS, các chương), viết nội dung chương 1–4, 6 | Claude |
| 3 | **Chạy thực nghiệm chính thức** trên máy của nhóm (cắm sạc, chế độ High Performance, tắt bớt ứng dụng nền) | Nhóm (Claude chạy thử trước) |
| 4 | Chương 5: biểu đồ và phân tích từ số liệu thật | Claude, sau khi có số liệu |
| 5 | Slide Beamer và tài liệu thuyết trình | Claude |
| 6 | Logo trường độ phân giải cao | Nhóm cung cấp |
| 7 | Rà soát nội dung, tập thuyết trình theo thời gian | Nhóm |
