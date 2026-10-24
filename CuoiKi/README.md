# Cuối kỳ – Parallel Prefix Sum Using OpenMP: Implementation and Performance Evaluation

Môn **Tính toán song song** – Nhóm: Bùi Xuân Chung, Lê Văn Cường, Đào Ngọc Cường.

## Cấu trúc thư mục

| Thư mục | Nội dung |
|---|---|
| `TaiLieu/` | Tài liệu tham khảo và bản tóm tắt ([TomTat_TaiLieu.md](TaiLieu/TomTat_TaiLieu.md)) |
| `src/` | Mã nguồn C/OpenMP các phiên bản scan |
| `scripts/` | Script build, chạy benchmark, vẽ biểu đồ |
| `results/` | Kết quả đo (CSV) và biểu đồ |
| `report/` | Báo cáo và slide |

## Tài liệu tham khảo
1. G. E. Blelloch, *Prefix Sums and Their Applications*, CMU-CS-90-190, 1990. https://www.cs.cmu.edu/~scandal/papers/CMU-CS-90-190.html
2. M. Harris, S. Sengupta, J. D. Owens, *Parallel Prefix Sum (Scan) with CUDA*, GPU Gems 3, Ch. 39. https://developer.nvidia.com/gpugems/GPUGems3/gpugems3_ch39.html

## Môi trường
- CPU: Intel Core i5-1335U (10 nhân / 12 luồng: 2 P-core có Hyper-Threading và 8 E-core)
- Biên dịch: GCC 15.2 + OpenMP trong WSL Ubuntu 26.04 (`gcc -O2 -fopenmp`). Đã kiểm tra chạy được 12 luồng, hỗ trợ cả `#pragma omp scan` (OpenMP 5.0).
- Kiểm tra môi trường: `gcc -O2 -fopenmp scripts/check_openmp.c -o check && ./check`

> Trạng thái: đang chuẩn bị, chờ yêu cầu chi tiết.
