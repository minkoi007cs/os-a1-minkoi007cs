# Verification - 2026-10-13

- Clean build: `make clean`, then `make -j4 kernel/kernel fs.img` completed successfully, with no compiler or linker warnings.
- Assignment checks: `python3 tests/functional.py` completed with `ALL A1 FUNCTIONAL TESTS PASSED`.
- Final xv6 regression suite: `python3 test-xv6.py -q usertests` completed with `ALL TESTS PASSED`.
- Covered strict argument validation and exit statuses; 52-level traversal; wildcard placement and type filters; root/trailing slashes; xargs batching, empty input, EOF, 32 arguments, long words and child failures; sysinfo memory/process/file counts; getprocs limits, bad/lazy destinations, parent IDs and concurrent fork/exit.
- Trace output was checked for self-tracing, signed arguments/results, bad path pointers, fork inheritance, saved paths across exec, nonreturning exit and reset on reused slots. `ps` was checked for literal tab separators and prescribed state names.

The supplied starter does not include `grade-a1`. The Oct 10 Canvas notice says to ignore `make grade`; these are independent checks, not official grades.

## Kết quả bằng tiếng Việt

Mã bài tập đã hoàn thành. Build sạch, kiểm tra riêng và `usertests -q` trên bản cuối đều đạt. Có kiểm tra cây sâu 52 tầng, input sai, giới hạn đối số, con trỏ không hợp lệ và nhiều process fork/exit đồng thời.

Theo yêu cầu của người dùng, phần nộp bài không được thực hiện: chưa gửi bài trên Canvas, chưa tạo tag `a1-submit`, và chưa khai số giờ vào `time.txt`. Những bước này thuộc khâu nộp; mã và kiểm thử đã sẵn sàng để học, xem lại và chạy tại máy.
