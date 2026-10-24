# Con trỏ (Pointer) trong C/C++

Báo cáo giữa kỳ môn **Tính toán song song**.

| Họ và tên | Mã sinh viên |
|---|---|
| Bùi Xuân Chung | |
| Lê Văn Cường | |
| Đào Ngọc Cường | |

## Nội dung

| File | Mô tả |
|---|---|
| [`slides/ConTro_C_CPP.pptx`](slides/ConTro_C_CPP.pptx) | Slide thuyết trình (13 slide, khoảng 10 phút) |
| [`ThuyetTrinh_ConTro.md`](ThuyetTrinh_ConTro.md) | Tài liệu cho người thuyết trình: lời nói, giải thích chi tiết, câu hỏi dự phòng |
| [`code/sum_pthread.c`](code/sum_pthread.c) | Ví dụ demo: tính tổng mảng bằng nhiều luồng (pthread) |
| [`tools/build_slides.py`](tools/build_slides.py) | Script sinh lại file slide (python-pptx) |

## Chạy demo

```bash
gcc code/sum_pthread.c -o sum -pthread
./sum
```

Trên Windows dùng WSL hoặc MinGW-w64 (có winpthreads).

## Sinh lại slide

```bash
pip install python-pptx
python tools/build_slides.py
```
