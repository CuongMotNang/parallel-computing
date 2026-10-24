# TÀI LIỆU THUYẾT TRÌNH – CON TRỎ (POINTER) TRONG C/C++

**Môn:** Tính toán song song – Báo cáo giữa kỳ
**Nhóm:** Bùi Xuân Chung – Lê Văn Cường – Đào Ngọc Cường
**Thời lượng:** khoảng 10 phút, 13 slide

> **Cách dùng tài liệu này**
> - Phần **🎤 Lời nói** là gợi ý câu nói trên lớp. Không cần đọc thuộc, nói theo ý là được.
> - Phần **📖 Giải thích thêm** để người trình bày **hiểu sâu**. Không bắt buộc nói hết, nhưng nên nắm để trả lời câu hỏi.
> - Phần **⚠️ Dễ bị hỏi** là những chỗ thầy/cô hay hỏi vặn.
> - Cuối tài liệu có mục **Câu hỏi dự phòng** cho phần hỏi đáp.

| Người trình bày | Slide | Thời gian |
|---|---|---|
| Bùi Xuân Chung | 1 → 5 | ~3,5 phút |
| Lê Văn Cường | 6 → 9 | ~3,5 phút |
| Đào Ngọc Cường | 10 → 13 | ~3 phút |

---

# PHẦN 1 – BÙI XUÂN CHUNG (Slide 1 → 5)

## Slide 1 – Trang bìa (~20 giây)

**🎤 Lời nói:**
> "Em chào thầy/cô và các bạn. Nhóm em gồm ba thành viên: Bùi Xuân Chung, Lê Văn Cường và Đào Ngọc Cường. Hôm nay nhóm em xin trình bày báo cáo giữa kỳ môn Tính toán song song với chủ đề **Con trỏ trong C/C++**."

---

## Slide 2 – Nội dung trình bày (~20 giây)

**🎤 Lời nói:**
> "Bài trình bày gồm các phần sau. Đầu tiên là bộ nhớ và địa chỉ, tiếp theo là khái niệm con trỏ, con trỏ với mảng, với hàm, cấp phát động, các lỗi thường gặp, con trỏ trong C++ hiện đại. Cuối cùng là phần gắn với môn học: **vai trò của con trỏ trong lập trình song song**."

**📖 Giải thích thêm:** Nên nhấn mạnh ngay từ đầu rằng bài có liên hệ với môn Tính toán song song, để thầy/cô thấy chủ đề không bị lạc môn.

---

## Slide 3 – Bộ nhớ và địa chỉ (~50 giây)

**🎤 Lời nói:**
> "Để hiểu con trỏ, trước hết phải hiểu bộ nhớ. RAM có thể hình dung như một dãy rất dài các ô nhớ. Mỗi ô có kích thước 1 byte và có một số thứ tự riêng, gọi là **địa chỉ**. Khi ta khai báo `int x = 10;`, chương trình dành cho x một vùng nhớ, thường là 4 byte, và lưu giá trị 10 vào đó. Toán tử `&` cho ta biết x đang nằm ở địa chỉ nào."

**📖 Giải thích thêm:**
- Mỗi biến có **4 thông tin**: **tên** (x), **kiểu** (int), **giá trị** (10) và **địa chỉ** (ví dụ `0x7ffe5c3a`).
- Bộ nhớ đánh địa chỉ **theo từng byte**. Biến `int` 4 byte chiếm 4 địa chỉ liên tiếp, và **địa chỉ của biến là địa chỉ byte đầu tiên**.
- Địa chỉ thường được in ở dạng **hệ 16 (hex)**, ví dụ `0x7ffe...`.
- `%p` là định dạng để in địa chỉ. Chuẩn C yêu cầu ép kiểu về `(void*)` khi in bằng `%p`, đó là lý do code có `(void*)&x`.
- **Mỗi lần chạy, địa chỉ in ra có thể khác nhau.** Hệ điều hành dùng cơ chế ASLR (ngẫu nhiên hóa vị trí bộ nhớ) để tăng bảo mật. Đây là hiện tượng bình thường, không phải lỗi.
- Biến cục bộ trong hàm nằm ở vùng **Stack**, còn bộ nhớ cấp phát động nằm ở vùng **Heap** (sẽ nói ở slide 7).

**⚠️ Dễ bị hỏi:** *"Sao chạy lại địa chỉ khác?"* → Do ASLR, như giải thích ở trên.

---

## Slide 4 – Con trỏ là gì? (~60 giây)

**🎤 Lời nói:**
> "Con trỏ là một biến đặc biệt. Thay vì lưu số hay ký tự, nó **lưu địa chỉ của một biến khác**. Ở ví dụ này, `int *p = &x;` nghĩa là p lưu địa chỉ của x, hay nói cách khác là p *trỏ tới* x. Khi viết `*p`, ta đi theo địa chỉ đó để truy cập chính biến x. Vì vậy `*p = 20` thực chất là gán x bằng 20, và in x ra sẽ được 20."

**📖 Giải thích thêm:**
- Hai toán tử cần nhớ:
  | Toán tử | Tên | Ý nghĩa |
  |---|---|---|
  | `&x` | lấy địa chỉ | trả về địa chỉ của x |
  | `*p` | giải tham chiếu (dereference) | truy cập giá trị tại địa chỉ p đang giữ |
- **Dấu `*` có hai nghĩa khác nhau:**
  - Trong **khai báo** (`int *p;`), `*` báo rằng p là con trỏ.
  - Trong **biểu thức** (`*p = 20;`), `*` là phép truy cập giá trị.
- **Bản thân con trỏ cũng là một biến**, nên nó cũng có địa chỉ riêng (`&p`). Từ đó có khái niệm con trỏ cấp 2 (`int **pp = &p;`).
- **Kích thước con trỏ không phụ thuộc kiểu nó trỏ tới.** Trên máy 64-bit, `int*`, `char*`, `double*` đều là **8 byte**, trên máy 32-bit là 4 byte.
- **Vậy tại sao con trỏ vẫn cần kiểu?** Vì có hai lý do:
  1. Khi `*p`, máy cần biết phải đọc bao nhiêu byte (int đọc 4, double đọc 8).
  2. Số học con trỏ `p + 1` cần biết phải nhảy bao nhiêu byte (slide 5).
- Bẫy cú pháp: `int *p, q;` chỉ có **p là con trỏ**, còn q là `int` thường.

**⚠️ Dễ bị hỏi:**
- *"sizeof(int*) bằng bao nhiêu?"* → 8 byte trên hệ 64-bit.
- *"`p`, `*p`, `&p` khác nhau thế nào?"* → `p` là địa chỉ của x, `*p` là giá trị của x (20), còn `&p` là địa chỉ của chính biến p.

---

## Slide 5 – Con trỏ và mảng (~50 giây)

**🎤 Lời nói:**
> "Trong C, mảng và con trỏ liên quan rất chặt. Khi dùng tên mảng `a` trong biểu thức, nó tự chuyển thành địa chỉ phần tử đầu tiên, tức là `&a[0]`. Do đó ta gán được `int *p = a;`. Phép `p + 1` không cộng thêm 1 byte mà nhảy sang **phần tử kế tiếp**, tức là cộng thêm `sizeof(int)` = 4 byte. Vì vậy `a[i]` thực chất chính là `*(a + i)`."

**📖 Giải thích thêm:**
- **Các phần tử mảng nằm liên tiếp trong bộ nhớ.** Đây là lý do số học con trỏ dùng được với mảng.
- Minh họa với `int a[5]` bắt đầu ở địa chỉ 1000:
  | Biểu thức | Địa chỉ | Giá trị |
  |---|---|---|
  | `a` hoặc `p` | 1000 | `*p` = 1 |
  | `p + 1` | 1004 | `*(p+1)` = 2 |
  | `p + 2` | 1008 | `*(p+2)` = 3 |
- Công thức tổng quát: `p + k` = địa chỉ của p **+ k × sizeof(kiểu)**.
- Các phép toán hợp lệ với con trỏ:
  - cộng/trừ một số nguyên (`p + 2`, `p--`)
  - **trừ hai con trỏ cùng mảng**, kết quả là **số phần tử** giữa chúng, không phải số byte
  - so sánh (`p < a + 5`)
  - **không được** cộng hai con trỏ với nhau, nhân hay chia con trỏ.
- **Mảng không hoàn toàn giống con trỏ:**
  - `sizeof(a)` = 20 (cả mảng 5×4 byte), còn `sizeof(p)` = 8 (chỉ là con trỏ).
  - Không gán lại được tên mảng (`a = ...` báo lỗi), còn con trỏ thì gán lại được.
- **C không kiểm tra biên mảng.** Truy cập `a[10]` với mảng 5 phần tử vẫn biên dịch được nhưng là **hành vi không xác định** (undefined behavior), có thể đọc ra rác hoặc làm crash chương trình.
- **Liên hệ với song song:** vì mảng liên tục và có số học con trỏ, ta có thể **chia mảng thành nhiều đoạn** chỉ bằng `a + offset` mà không cần sao chép. Ý này sẽ dùng ở slide 11.

**⚠️ Dễ bị hỏi:** *"Mảng có phải là con trỏ không?"* → **Không.** Tên mảng chỉ *được chuyển thành* con trỏ khi dùng trong biểu thức. Bằng chứng là `sizeof` cho kết quả khác nhau và tên mảng không gán lại được.

**➡️ Chuyển người:** *"Tiếp theo, bạn Lê Văn Cường sẽ trình bày về con trỏ với hàm và quản lý bộ nhớ."*

---

# PHẦN 2 – LÊ VĂN CƯỜNG (Slide 6 → 9)

## Slide 6 – Con trỏ và hàm (~50 giây)

**🎤 Lời nói:**
> "Trong C, khi gọi hàm, tham số được **truyền theo giá trị**, tức là hàm chỉ nhận một bản sao. Nếu viết hàm swap nhận `int a, int b` thì hàm chỉ hoán đổi hai bản sao, còn biến gốc bên ngoài vẫn giữ nguyên. Muốn hàm sửa được biến gốc, ta truyền **địa chỉ** của biến: `swap(&x, &y)`. Bên trong hàm, `*a` và `*b` chính là x và y thật."

**📖 Giải thích thêm:**
- So sánh hai cách viết:
  ```c
  void swap_sai(int a, int b) { int t = a; a = b; b = t; }      // không có tác dụng
  void swap(int *a, int *b)   { int t = *a; *a = *b; *b = t; }  // đúng
  ```
- Thực chất **con trỏ vẫn được truyền theo giá trị**: hàm nhận bản sao của *địa chỉ*. Nhưng bản sao địa chỉ vẫn trỏ đúng tới biến gốc, nên sửa được biến gốc.
- **Ba công dụng chính của con trỏ khi làm tham số:**
  1. **Cho hàm sửa biến gốc**, ví dụ swap hoặc `scanf("%d", &x)`. Đây là lý do `scanf` cần dấu `&`.
  2. **Trả về nhiều kết quả**: một hàm có thể ghi kết quả vào nhiều con trỏ đầu ra.
  3. **Truyền dữ liệu lớn hiệu quả**: truyền mảng 1 triệu phần tử chỉ tốn 8 byte (một địa chỉ), không phải sao chép cả mảng.
- Khi truyền mảng vào hàm, `void f(int a[])` thực chất là `void f(int *a)`. Vì vậy **hàm không biết độ dài mảng** và phải truyền thêm tham số `n`.
- Nếu hàm chỉ đọc, không sửa, nên dùng `const int *a` để trình biên dịch ngăn sửa nhầm.
- **Con trỏ hàm** lưu địa chỉ của một hàm, dùng để truyền hàm làm tham số (callback). Ví dụ có sẵn là `qsort` trong C nhận con trỏ tới hàm so sánh. Ở slide 10, `pthread_create` cũng nhận con trỏ tới hàm mà luồng sẽ chạy.

**⚠️ Dễ bị hỏi:**
- *"Tại sao scanf cần &?"* → Vì scanf phải ghi giá trị vào biến của ta, nên cần biết địa chỉ biến.
- *"Không được trả về con trỏ tới biến cục bộ, vì sao?"* → Biến cục bộ nằm trên Stack và bị hủy khi hàm kết thúc, nên con trỏ trả về sẽ thành dangling pointer.

---

## Slide 7 – Cấp phát bộ nhớ động (~55 giây)

**🎤 Lời nói:**
> "Có những lúc phải đến khi chạy chương trình mới biết cần bao nhiêu bộ nhớ, ví dụ người dùng nhập n phần tử. Lúc đó ta xin bộ nhớ trên vùng **Heap**. Trong C dùng `malloc` để cấp phát và `free` để giải phóng. Trong C++ dùng `new` và `delete`. Hàm cấp phát trả về **một con trỏ** tới vùng nhớ vừa xin, vì vậy cấp phát động không thể tách khỏi con trỏ. Điểm quan trọng là bộ nhớ Heap **không tự giải phóng**, lập trình viên phải tự giải phóng."

**📖 Giải thích thêm:**
- **So sánh Stack và Heap:**
  | | Stack | Heap |
  |---|---|---|
  | Chứa gì | biến cục bộ, tham số hàm | bộ nhớ cấp phát động |
  | Quản lý | tự động (hết hàm là giải phóng) | lập trình viên tự quản lý |
  | Kích thước | nhỏ (thường 1–8 MB) | lớn (gần bằng RAM còn trống) |
  | Tốc độ cấp phát | rất nhanh | chậm hơn |
- Ví dụ: `int a[10000000];` khai báo cục bộ có thể gây **stack overflow**, nên mảng lớn phải dùng `malloc`/`new`.
- **Các hàm trong C** (thư viện `<stdlib.h>`):
  - `malloc(size)`: cấp `size` byte, **giá trị bên trong là rác**.
  - `calloc(n, size)`: cấp n phần tử và **gán tất cả về 0**.
  - `realloc(p, new_size)`: thay đổi kích thước vùng đã cấp.
  - `free(p)`: giải phóng.
- **Luôn kiểm tra kết quả cấp phát**, vì malloc trả về `NULL` nếu hết bộ nhớ:
  ```c
  int *arr = malloc(n * sizeof(int));
  if (arr == NULL) { printf("Het bo nho\n"); return 1; }
  ```
- Trong C, việc ép kiểu `(int*)` trước malloc là **không bắt buộc**. Trong C++ thì **bắt buộc**, nhưng C++ nên dùng `new`.
- **Trong C++:**
  - `new int` đi với `delete`, còn `new int[n]` đi với **`delete[]`**. Dùng sai cặp là lỗi.
  - `new` ném ngoại lệ `std::bad_alloc` khi thiếu bộ nhớ thay vì trả về NULL.
  - `new`/`delete` gọi constructor/destructor của đối tượng, còn malloc/free thì không.
- **Không được trộn lẫn**: vùng nhớ cấp bằng malloc thì giải phóng bằng free, cấp bằng new thì giải phóng bằng delete.

**⚠️ Dễ bị hỏi:**
- *"malloc khác calloc thế nào?"* → calloc gán giá trị về 0, và nhận tham số dạng (số phần tử, kích thước).
- *"new khác malloc thế nào?"* → new là toán tử của C++, gọi constructor, không cần ép kiểu và ném exception khi lỗi.

---

## Slide 8 – Lỗi thường gặp với con trỏ (~55 giây)

**🎤 Lời nói:**
> "Con trỏ mạnh nhưng cũng dễ gây lỗi. Có bốn lỗi điển hình. Thứ nhất là con trỏ chưa khởi tạo, nó trỏ tới một địa chỉ ngẫu nhiên. Thứ hai là truy cập con trỏ NULL, chương trình sẽ crash. Thứ ba là dangling pointer, tức là vẫn dùng con trỏ sau khi vùng nhớ đã bị giải phóng. Thứ tư là rò rỉ bộ nhớ, tức là cấp phát mà quên giải phóng. Cách phòng tránh là luôn khởi tạo con trỏ và gán về NULL sau khi free."

**📖 Giải thích thêm (ví dụ cho từng lỗi):**
1. **Wild pointer (con trỏ hoang):**
   ```c
   int *p;      // p chứa giá trị rác
   *p = 5;      // ghi vào địa chỉ ngẫu nhiên → crash hoặc phá dữ liệu khác
   ```
2. **NULL pointer dereference:**
   ```c
   int *p = NULL;
   *p = 5;      // Segmentation fault
   ```
   Lỗi này **dễ phát hiện** vì chương trình crash ngay. Đây là lý do nên khởi tạo `= NULL`: lỗi sẽ lộ ra sớm thay vì âm thầm phá dữ liệu.
3. **Dangling pointer:**
   ```c
   int *p = malloc(sizeof(int));
   free(p);
   *p = 5;      // vùng nhớ đã trả lại hệ thống → hành vi không xác định
   ```
   Lỗi này **nguy hiểm nhất** vì nhiều khi chương trình vẫn chạy "bình thường" rồi mới sai ở chỗ khác. Lỗi **double free** (free hai lần) cũng thuộc nhóm này.
4. **Memory leak (rò rỉ bộ nhớ):**
   ```c
   int *p = malloc(100 * sizeof(int));
   p = malloc(200 * sizeof(int));   // mất địa chỉ vùng 100 phần tử, không free được nữa
   ```
   Chương trình chạy lâu như server sẽ bị tốn RAM dần cho đến khi hết bộ nhớ.
- **Thói quen tốt:**
  - Khai báo là khởi tạo ngay: `int *p = NULL;` (C++ dùng `nullptr`).
  - Sau `free(p);` thì gán `p = NULL;`. Gọi `free(NULL)` là an toàn, nên tránh được double free.
  - Mỗi lần malloc phải có một lần free tương ứng.
  - Dùng công cụ kiểm tra: **Valgrind** (Linux) hoặc **AddressSanitizer** (`gcc -fsanitize=address`).

**⚠️ Dễ bị hỏi:** *"Segmentation fault là gì?"* → Là lỗi khi chương trình truy cập vùng nhớ nó không có quyền truy cập. Hệ điều hành sẽ dừng chương trình.

---

## Slide 9 – Con trỏ trong C++ hiện đại (~50 giây)

**🎤 Lời nói:**
> "Vì con trỏ thường dễ gây lỗi, C++ cung cấp các công cụ an toàn hơn. Thứ nhất là **tham chiếu**, một tên gọi khác của biến, giúp hàm sửa biến gốc mà không cần dùng dấu `*`. Thứ hai là **smart pointer**, loại con trỏ thông minh tự giải phóng bộ nhớ khi không còn dùng. `unique_ptr` chỉ có một chủ sở hữu duy nhất, còn `shared_ptr` cho phép nhiều chủ sở hữu và dùng bộ đếm để biết khi nào cần giải phóng."

**📖 Giải thích thêm:**
- **Tham chiếu (reference):**
  ```cpp
  void swap(int &a, int &b) { int t = a; a = b; b = t; }
  swap(x, y);   // không cần &x, &y
  ```
  | | Con trỏ | Tham chiếu |
  |---|---|---|
  | Có thể NULL | có | không |
  | Gán lại sang biến khác | được | không (gắn cố định từ lúc khởi tạo) |
  | Bắt buộc khởi tạo | không | có |
  | Cú pháp | dùng `*`, `&` | như biến thường |
- **Smart pointer** hoạt động theo nguyên tắc **RAII**: khi đối tượng smart pointer ra khỏi phạm vi (scope), destructor của nó **tự gọi delete**. Nhờ vậy không quên giải phóng, kể cả khi có exception.
- **`unique_ptr`:**
  - Chỉ **một** con trỏ được sở hữu vùng nhớ.
  - **Không sao chép được**, chỉ **chuyển quyền sở hữu** bằng `std::move`.
  - Không tốn thêm chi phí so với con trỏ thường. Đây là **lựa chọn mặc định**.
- **`shared_ptr`:**
  - Nhiều con trỏ cùng sở hữu một vùng nhớ.
  - Có **bộ đếm tham chiếu** (reference count): mỗi lần sao chép thì đếm tăng, mỗi lần hủy thì đếm giảm, **khi đếm về 0 thì tự delete**.
  - Bộ đếm được tăng/giảm theo kiểu **nguyên tử (atomic)**, nên an toàn khi sao chép shared_ptr giữa nhiều luồng. Đây là điểm liên hệ với môn học. Lưu ý: chỉ *bộ đếm* là an toàn, còn *dữ liệu được trỏ tới* vẫn cần đồng bộ.
- **`weak_ptr`** (chỉ nhắc nếu được hỏi): quan sát shared_ptr mà không tăng bộ đếm, dùng để tránh **vòng tham chiếu** (A trỏ B, B trỏ A thì bộ đếm không bao giờ về 0, gây leak).
- Nên dùng `make_unique` / `make_shared` thay vì `new` trực tiếp.

**⚠️ Dễ bị hỏi:**
- *"Đã có smart pointer thì con trỏ thường còn dùng làm gì?"* → Dùng để *quan sát* mà không sở hữu, để làm việc với thư viện C, và trong code cần hiệu năng cao ở mức thấp.
- *"unique_ptr khác shared_ptr?"* → unique_ptr có một chủ sở hữu, không copy được. shared_ptr có nhiều chủ sở hữu và dùng bộ đếm.

**➡️ Chuyển người:** *"Phần cuối, bạn Đào Ngọc Cường sẽ trình bày về con trỏ trong tính toán song song."*

---

# PHẦN 3 – ĐÀO NGỌC CƯỜNG (Slide 10 → 13)

## Slide 10 – Con trỏ trong tính toán song song (~60 giây)

**🎤 Lời nói:**
> "Đây là phần gắn với môn học. Trong mô hình **bộ nhớ chia sẻ** (shared memory), các luồng của cùng một tiến trình dùng chung một không gian địa chỉ. Vì vậy con trỏ là cách tự nhiên để chia dữ liệu: thay vì sao chép dữ liệu cho từng luồng, ta chỉ cần đưa cho mỗi luồng **địa chỉ đoạn dữ liệu nó phải xử lý**. Thư viện POSIX Threads, gọi tắt là pthread, truyền tham số cho luồng qua một con trỏ kiểu `void *`. Tuy nhiên chia sẻ bộ nhớ cũng kéo theo hai rủi ro chính: race condition và truyền địa chỉ không còn hợp lệ."

**📖 Giải thích thêm:**
- **Tiến trình (process) và luồng (thread):**
  - Mỗi tiến trình có **không gian bộ nhớ riêng**, nên con trỏ của tiến trình này không dùng được ở tiến trình khác.
  - Các luồng trong cùng tiến trình **dùng chung Heap, biến toàn cục và mã lệnh**, nhưng **mỗi luồng có Stack riêng**.
  - Vì vậy một con trỏ tới Heap hoặc biến toàn cục có nghĩa như nhau ở mọi luồng.
- **Hàm tạo luồng:**
  ```c
  int pthread_create(pthread_t *thread, const pthread_attr_t *attr,
                     void *(*start_routine)(void *), void *arg);
  ```
  Trong một hàm này có đủ các loại con trỏ:
  - `pthread_t *thread`: con trỏ để hàm **ghi ra** mã luồng (giống ý ở slide 6).
  - `void *(*start_routine)(void *)`: **con trỏ hàm**, là hàm mà luồng sẽ chạy.
  - `void *arg`: **con trỏ void**, chứa tham số truyền cho luồng.
- **Tại sao lại là `void *`?** Vì `void *` là "con trỏ chung", có thể trỏ tới **bất kỳ kiểu nào**. Thư viện không biết ta muốn truyền gì nên dùng void*. Bên trong hàm luồng, ta **ép kiểu ngược lại** về kiểu thật, ví dụ `Task *t = (Task *)arg;`. Muốn truyền nhiều tham số thì gói chúng vào một `struct` rồi truyền địa chỉ struct.
- **Rủi ro 1 – Race condition (tranh chấp dữ liệu):**
  ```c
  long total = 0;               // biến dùng chung
  // trong mỗi luồng:
  total += x;                   // KHÔNG an toàn
  ```
  `total += x` thực ra gồm 3 bước: **đọc, cộng, ghi**. Hai luồng có thể cùng đọc giá trị cũ, rồi luồng ghi sau đè lên luồng ghi trước, làm mất một phần kết quả. Cách khắc phục:
  - Dùng **mutex** (`pthread_mutex_lock/unlock`) để chỉ một luồng được sửa tại một thời điểm.
  - Dùng **biến nguyên tử** (atomic).
  - **Tốt nhất là tránh chia sẻ**: mỗi luồng ghi vào ô riêng, cuối cùng luồng chính mới cộng lại. Ví dụ ở slide 11 dùng cách này.
- **Rủi ro 2 – Truyền địa chỉ không còn hợp lệ:**
  - Lỗi kinh điển:
    ```c
    for (int i = 0; i < T; i++)
        pthread_create(&th[i], NULL, work, &i);   // SAI
    ```
    Tất cả các luồng cùng nhận **địa chỉ của một biến i**, mà i liên tục thay đổi trong vòng lặp. Khi luồng kịp đọc thì i có thể đã thành giá trị khác. **Cách sửa:** mỗi luồng một phần tử riêng trong mảng (như `&tk[i]` ở slide 11).
  - Tương tự, không truyền địa chỉ biến cục bộ của một hàm rồi để hàm đó kết thúc trước khi luồng chạy xong, vì con trỏ sẽ thành dangling pointer.
- **Liên hệ mở rộng (nhắc nếu còn thời gian):**
  - **OpenMP**: các luồng cũng dùng chung mảng thông qua con trỏ, ví dụ `#pragma omp parallel for` trên mảng `a`.
  - **CUDA (GPU)**: `cudaMalloc` trả về con trỏ tới bộ nhớ GPU. CPU và GPU có bộ nhớ riêng, nên phải sao chép dữ liệu qua lại bằng `cudaMemcpy`.
  - **MPI** (bộ nhớ phân tán): mỗi tiến trình có bộ nhớ riêng nên **không chia sẻ con trỏ được**, phải gửi dữ liệu bằng thông điệp. Tuy vậy các hàm như `MPI_Send` vẫn nhận con trỏ tới bộ đệm dữ liệu.
  - **False sharing**: nhiều luồng ghi vào các biến khác nhau nhưng nằm chung một cache line (64 byte), làm chương trình chậm đi dù kết quả vẫn đúng.

**⚠️ Dễ bị hỏi:**
- *"void* là gì, có dereference trực tiếp được không?"* → Không. Phải ép về kiểu cụ thể trước, vì máy không biết cần đọc bao nhiêu byte.
- *"Làm sao tránh race condition?"* → Dùng mutex, atomic, hoặc thiết kế để mỗi luồng ghi vào vùng riêng.

---

## Slide 11 – Ví dụ: Tính tổng mảng bằng nhiều luồng (~60 giây)

**🎤 Lời nói:**
> "Đây là ví dụ tính tổng mảng 8 phần tử bằng 2 luồng. Mỗi luồng nhận một struct `Task` gồm **con trỏ `start` trỏ tới đầu đoạn mảng của nó**, độ dài đoạn, và ô `sum` để ghi kết quả. Dòng quan trọng nhất là `tk[i].start = a + i * (N / T);`. Đây là số học con trỏ: luồng 0 nhận `a + 0`, tức là từ phần tử thứ nhất, còn luồng 1 nhận `a + 4`, tức là từ phần tử thứ năm. Mảng **không bị sao chép**, cả hai luồng cùng làm việc trên một mảng gốc. Mỗi luồng ghi kết quả vào `sum` của riêng nó nên **không có race condition**. Cuối cùng luồng chính chờ các luồng xong bằng `pthread_join` rồi cộng lại, được kết quả 36."

**📖 Giải thích từng phần của code:**
| Dòng code | Ý nghĩa |
|---|---|
| `int *start;` trong struct | con trỏ tới đầu đoạn mảng mà luồng xử lý |
| `void *work(void *arg)` | hàm luồng, đúng mẫu pthread yêu cầu (nhận và trả về `void *`) |
| `Task *t = (Task *)arg;` | ép `void *` về kiểu thật `Task *` |
| `t->sum` | toán tử `->` là truy cập thành viên qua con trỏ, tương đương `(*t).sum` |
| `*(t->start + i)` | phần tử thứ i của đoạn, tương đương `t->start[i]` |
| `a + i * (N / T)` | số học con trỏ, tính địa chỉ đầu đoạn thứ i |
| `&tk[i]` | **mỗi luồng một địa chỉ riêng**, tránh lỗi `&i` đã nói ở slide 10 |
| `pthread_join` | chờ luồng chạy xong rồi mới đọc `sum`, nếu không sẽ đọc kết quả chưa tính xong |

- **Phân chia công việc:**
  | Luồng | `start` | Phần tử xử lý | `sum` |
  |---|---|---|---|
  | 0 | `a + 0` | 1, 2, 3, 4 | 10 |
  | 1 | `a + 4` | 5, 6, 7, 8 | 26 |
  | | | **Tổng** | **36** |
- **Hạn chế của code mẫu** (nếu bị hỏi):
  - Code giả sử N chia hết cho T. Nếu không chia hết, luồng cuối phải nhận thêm phần dư: `len = N - i*(N/T)`.
  - Mảng 8 phần tử thì chạy song song **không nhanh hơn**, vì chi phí tạo luồng lớn hơn phần tính toán. Song song chỉ có lợi với dữ liệu lớn (hàng triệu phần tử).
  - Các `sum` của mỗi luồng nằm sát nhau trong mảng `tk`, nên có thể bị **false sharing**. Với ví dụ nhỏ thì không đáng kể.
- **Cách chạy demo:**
  - Linux/WSL: `gcc sum_pthread.c -o sum -pthread` rồi `./sum`
  - Windows: cần MinGW-w64 có hỗ trợ pthread (bản winpthreads), lệnh tương tự.

**⚠️ Dễ bị hỏi:**
- *"Nếu dùng một biến total chung thì sao?"* → Bị race condition (slide 10), phải thêm mutex. Cách của nhóm tránh được việc đó mà không cần khóa.
- *"Tại sao không truyền `&i`?"* → Vì mọi luồng sẽ dùng chung một biến i đang thay đổi.

---

## Slide 12 – Kết luận (~30 giây)

**🎤 Lời nói:**
> "Tóm lại, con trỏ cho phép thao tác trực tiếp với bộ nhớ. Đây là nền tảng của mảng, truyền tham số, cấp phát động và các cấu trúc dữ liệu. Trong tính toán song song, con trỏ giúp các luồng chia nhau dữ liệu mà không phải sao chép, nhờ đó tiết kiệm bộ nhớ và thời gian. Tuy nhiên con trỏ dễ gây lỗi bộ nhớ và lỗi tranh chấp dữ liệu, nên cần dùng cẩn thận. Trong C++ nên ưu tiên tham chiếu và smart pointer."

---

## Slide 13 – Cảm ơn và Hỏi đáp (~10 giây)

**🎤 Lời nói:**
> "Phần trình bày của nhóm em đến đây là kết thúc. Cảm ơn thầy/cô và các bạn đã lắng nghe. Nhóm em rất mong nhận được câu hỏi và góp ý."

---

# PHỤ LỤC A – CÂU HỎI DỰ PHÒNG (Hỏi đáp)

| # | Câu hỏi | Trả lời ngắn |
|---|---|---|
| 1 | Con trỏ chiếm bao nhiêu byte? | 8 byte trên hệ 64-bit, 4 byte trên 32-bit, không phụ thuộc kiểu trỏ tới. |
| 2 | `NULL` và `nullptr` khác nhau thế nào? | `NULL` thường chỉ là macro số 0, có thể gây nhầm khi chọn hàm nạp chồng. `nullptr` (C++11) có kiểu riêng dành cho con trỏ nên an toàn hơn. |
| 3 | Con trỏ cấp 2 (`int **`) dùng khi nào? | Khi hàm cần sửa *chính con trỏ* (ví dụ cấp phát bên trong hàm), và khi làm mảng 2 chiều động. |
| 4 | `const int *p` khác `int * const p`? | `const int *p`: không sửa được giá trị qua p, nhưng p trỏ sang chỗ khác được. `int * const p`: p cố định một chỗ, nhưng sửa được giá trị. Mẹo: đọc từ phải sang trái. |
| 5 | Tại sao Java/Python không có con trỏ? | Có, nhưng bị ẩn đi dưới dạng tham chiếu, và có bộ thu gom rác (GC) tự giải phóng. Đổi lại thì kém kiểm soát và hiệu năng thấp hơn. |
| 6 | Tại sao lập trình song song hiệu năng cao (HPC) vẫn dùng C/C++ và con trỏ? | Vì kiểm soát được bố cục bộ nhớ, tránh sao chép dữ liệu, không có GC làm gián đoạn, và các thư viện như OpenMP, MPI, CUDA đều dựa trên C/C++. |
| 7 | Race condition và deadlock khác gì nhau? | Race condition: nhiều luồng cùng truy cập dữ liệu chung, cho kết quả sai. Deadlock: các luồng chờ khóa của nhau mãi mãi nên chương trình bị treo. |
| 8 | Mutex là gì? | Là khóa loại trừ lẫn nhau: chỉ một luồng giữ khóa tại một thời điểm, đảm bảo đoạn code găng (critical section) không bị chạy đồng thời. |
| 9 | Có truyền con trỏ từ máy này sang máy khác (MPI) được không? | Không. Địa chỉ chỉ có nghĩa trong không gian bộ nhớ của một tiến trình, phải gửi *dữ liệu* chứ không gửi *địa chỉ*. |
| 10 | Undefined behavior là gì? | Là hành vi mà chuẩn C không quy định kết quả, ví dụ truy cập ngoài mảng hay dùng con trỏ đã free. Chương trình có thể chạy đúng, sai hoặc crash, và kết quả không đoán trước được. |
| 11 | `p++` và `(*p)++` khác nhau? | `p++` dịch con trỏ sang phần tử kế. `(*p)++` tăng giá trị tại chỗ p trỏ. Còn `*p++` là lấy giá trị rồi mới dịch con trỏ. |
| 12 | shared_ptr có an toàn đa luồng không? | Bộ đếm tham chiếu thì an toàn (atomic). Dữ liệu được trỏ tới thì **không**, vẫn cần mutex nếu nhiều luồng cùng ghi. |

---

# PHỤ LỤC B – MẸO KHI THUYẾT TRÌNH

- **Canh thời gian:** người 1 xong slide 5 ở khoảng phút 3:30, người 2 xong slide 9 ở khoảng phút 7:00. Nếu trễ thì người 3 **bỏ phần liên hệ mở rộng** ở slide 10 và chỉ nói ý chính của slide 11.
- **Khi giải thích code:** chỉ vào **1–2 dòng quan trọng nhất**, không đọc từng dòng.
- **Demo (nếu có):** chuẩn bị sẵn terminal đã biên dịch, chỉ cần chạy `./sum`. Nên chụp màn hình kết quả để dự phòng khi máy chiếu hoặc máy tính gặp sự cố.
- **Không biết câu trả lời:** nói thật là *"Phần này nhóm em chưa tìm hiểu sâu, nhóm em xin ghi nhận và sẽ bổ sung"*. Không nên đoán bừa.
- **Hình vẽ nên có trên slide:** ô nhớ với địa chỉ (slide 3), mũi tên p → x (slide 4), mảng với các ô `p, p+1, p+2` (slide 5), mảng chia đôi cho 2 luồng (slide 11).





