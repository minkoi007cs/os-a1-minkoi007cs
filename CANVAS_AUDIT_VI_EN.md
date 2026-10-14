# Assignment 1 — Kiểm tra lần hai / Second audit

Ngày kiểm tra: 13/10/2026. Đã đọc lại toàn bộ 8 trang đề và các cập nhật liên quan trên Canvas.

**Kết quả:** chưa phát hiện lỗi trong phần triển khai. Build sạch, bộ test mở rộng và `usertests -q` đều đạt. Lần rà soát này bổ sung kiểm thử và sửa hướng dẫn nộp; không thay đổi mã triển khai các chức năng bài tập.

**Result:** no implementation defect was found. The clean build, extended assignment checks, and xv6 regression suite passed. This audit changes tests and documentation, with no change to the assignment implementation.

## Nguồn đã đối chiếu / Sources reviewed

- [Assignment 1 trên Canvas](https://kent.instructure.com/courses/137884/assignments/2124766), gồm file “Assignment 1 System Calls and User Space in xv6.docx”, đủ 8 trang.
- [Thông báo 10/10](https://kent.instructure.com/courses/137884/discussion_topics/1243802): hạn mới 14/10; bỏ `make grade`; TA chấm mã thủ công.
- [Thông báo về AI](https://kent.instructure.com/courses/137884/discussion_topics/1222330): không bắt buộc mua gói AI. Đề cho phép dùng AI, người nộp chịu trách nhiệm về mã.
- [Modules](https://kent.instructure.com/courses/137884/modules) và [General Forum](https://kent.instructure.com/courses/137884/discussion_topics/1208433): không thấy yêu cầu bổ sung cho Assignment 1 trong nội dung hiện có.

## Đối chiếu yêu cầu / Requirements checked

| Phần / Item | Nội dung đã xác minh / Verified behavior |
|---|---|
| `sleep` | Gọi `pause`; số ticks chỉ gồm chữ số, nằm trong miền int; lỗi thiếu/thừa/âm/ký tự sai ghi đúng `usage: sleep <ticks>` vào stderr và exit 1; thành công exit 0. Test đo ticks riêng cho `sleep 2`. |
| `find` | Đường dẫn từ thư mục đầu; chỉ `*` là wildcard, kể cả chuỗi rỗng; bỏ root, `.` và `..`; `-type f` gồm T_FILE và T_DEVICE; `-type d` chỉ thư mục; lỗi mở thư mục đúng thông báo và exit 1. Kiểm tra tên 14 ký tự, dấu `?` literal, nhiều `*`, slash ở root/cuối và cây sâu 52 tầng. |
| `xargs` | Tách từ bằng space/tab/newline; mặc định mỗi dòng không rỗng; `-n` gom từ qua nhiều dòng; chạy tuần tự bằng fork/exec/wait; trả 1 nếu có child lỗi. Kiểm tra EOF, dòng rỗng, N sai, exactly 32 đối số, phần dư của batch, dòng quá giới hạn và từ dài 900 byte. |
| Syscall interface | `trace=23`, `sysinfo=24`, `getprocs=25`; có syscall table, handler, stub trong usys.pl, prototype và khai báo struct trong user.h. Các tiện ích được thêm vào UPROGS. |
| `trace` | Mask kế thừa qua fork, giữ qua exec, reset khi alloc/free; lưu a0 và path trước handler, kiểm tra mask sau handler. Đúng `[pid] name(arg) -> ret`, số dạng signed int, path có dấu ngoặc kép hoặc `?`, nhóm no-argument có `()`. Exit không in, trace tự trace được, trace(0) tắt được. |
| `sysinfo` | Đúng ba trường uint64; free list dưới kmem.lock; proc không UNUSED dưới p->lock; số entry ref > 0 dưới ftable.lock. Copyout không giữ các khóa. Kiểm tra cấp phát bộ nhớ, open/dup/pipe/close, fork/wait, zombie và con trỏ lỗi/read-only/lazy. |
| `getprocs` | Đúng ABI struct; đi theo thứ tự bảng, không vượt max; max âm/copy lỗi trả -1, max 0 không dereference. Lấy wait_lock trước p->lock; snapshot một row, nhả khóa trước copyout; không tạo mảng 64 row trên kernel stack. Kiểm tra parent PID, không ghi row tiếp theo khi max=1 và 500 lượt snapshot trong lúc 80 lượt fork/exit. |
| `ps` | Header chính xác `PID\tPPID\tSTATE\tSIZE\tNAME`; từng trường cách nhau một tab; state dùng đúng used/sleeping/runnable/running/zombie; SIZE in bằng `%lu`. |
| MAXARG | Sửa giới hạn mảng của starter để chứa đủ 32 đối số và NULL kết thúc; có test đúng ranh giới. |
| Build & history | `make clean` rồi build kernel/fs thành công, không cảnh báo; full starter history vẫn là ancestor của HEAD; `git diff --check` sạch. |

## Bằng chứng kiểm thử / Test evidence

Các lệnh đã chạy ở thư mục repository:

```sh
make clean
make -j4 kernel/kernel fs.img
python3 tests/functional.py
python3 test-xv6.py -q usertests
git diff --check
```

- Build: exit 0; không có compiler/linker warning.
- Bộ kiểm tra bài tập: 31 assertion của host runner đều đạt, cộng các assertion trong `a1test`; kết thúc bằng `ALL A1 FUNCTIONAL TESTS PASSED`. Số assertion nội bộ thay đổi theo lịch chạy process và số lần đọc pipe, không phải số test case cố định.
- Bộ regression xv6: `ALL TESTS PASSED`.
- `lazy_unmap` cố ý làm một child fault; dòng `usertrap` trong log là hành vi bài test đó kiểm tra, không phải kernel panic hay test thất bại.

Logs: `OS_Assignment_1_Functional_Tests.txt` và `OS_Assignment_1_Xv6_Regression.txt` trong cùng thư mục outputs với báo cáo này.

## Hoàn thiện mã và nộp bài / Implementation and submission status

Mã bài tập hoàn chỉnh trong phạm vi đã đối chiếu và kiểm thử. Ngày 13/10/2026, người học cung cấp thời gian thực tế là **2 giờ**. `time.txt` ở root được thêm với nội dung `2`; bản cuối được chuẩn bị để push lên GitHub và gắn tag `a1-submit`. Chưa nộp Canvas.

Đề yêu cầu repo public `os-a1-<username>`, giữ lịch sử starter, `time.txt` ở root và nộp duy nhất link repo. Tag `a1-submit` xác định commit muốn chấm; nếu thiếu tag, đề nói staff chấm commit cuối trên `main` đã push trước hạn. Hạn hiện tại: **14/10/2026, 11:59 PM giờ New York**.

Repo và bản ZIP cuối bao gồm mã triển khai, test mở rộng, báo cáo lần kiểm tra thứ hai và `time.txt`. Bản cuối được push lên `main` và tag `a1-submit` để xác định commit muốn chấm; Canvas chỉ cần link repository.

This is evidence of reviewed and tested behavior, not an official grade or a proof that no undiscovered defect can exist. The final grade depends on the TA's review.
