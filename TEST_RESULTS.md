# Verification - 2026-10-13

- Clean build: `make clean`, then `make -j4 kernel/kernel fs.img` completed successfully, with no compiler or linker warnings.
- Assignment checks: `python3 tests/functional.py` completed with `ALL A1 FUNCTIONAL TESTS PASSED`.
- Final xv6 regression suite: `python3 test-xv6.py -q usertests` completed with `ALL TESTS PASSED`.
- Covered strict argument validation and exit statuses; 52-level traversal; wildcard placement and type filters; root/trailing slashes; xargs batching, empty input, EOF, 32 arguments, long words and child failures; sysinfo memory/process/file counts; getprocs limits, bad/lazy destinations, parent IDs and concurrent fork/exit.
- Trace output was checked for self-tracing, signed arguments/results, bad path pointers, fork inheritance, saved paths across exec, nonreturning exit and reset on reused slots. `ps` was checked for literal tab separators and prescribed state names.

## Second audit against Canvas - 2026-10-13

Read all eight pages of the assignment again, the October 10 deadline/grading update, the AI clarification announcement, Modules, and the General Forum. No additional Assignment 1 specification was listed in Modules or the General Forum.

- Clean build repeated with no compiler/linker warnings.
- Extended `a1test` and `tests/functional.py` passed; the host runner reported 31 successful assertions and `ALL A1 FUNCTIONAL TESTS PASSED`.
- Added checks for T_DEVICE, full 14-character directory names, consecutive stars and literal question marks; xargs MAXARG plus a final partial batch, overfull default lines and `-n 1`; read-only destinations, truncation without overwriting the next record, and zombie accounting.
- Tested quoted paths and bad path pointers for every path-first syscall, empty argument lists for getpid/uptime/sync, trace(0), and a negative all-bits mask.
- Tightened the sleep timing test to start measuring immediately before sleep 2.
- Repeated the full `usertests -q` regression suite: `ALL TESTS PASSED`.
- No implementation defect was found in this audit. Changes are tests and documentation; the assignment implementation is unchanged.
- Corrected the submission documentation: the handout says that without `a1-submit`, staff grade the last commit on main pushed before the deadline. The requested tag is still the recommended submission path.

These are independent checks, not an official grade or a guarantee that every possible hidden case has been covered.

The supplied starter does not include `grade-a1`. The Oct 10 Canvas notice says to ignore `make grade`; these are independent checks, not official grades.

## Kết quả bằng tiếng Việt

Mã bài tập đã hoàn thành. Build sạch, kiểm tra riêng và `usertests -q` trên bản cuối đều đạt. Có kiểm tra cây sâu 52 tầng, input sai, giới hạn đối số, con trỏ không hợp lệ và nhiều process fork/exit đồng thời.

Theo yêu cầu của người dùng, phần nộp bài không được thực hiện: chưa gửi bài trên Canvas, chưa tạo tag `a1-submit`, và chưa khai số giờ vào `time.txt`. Những bước này thuộc khâu nộp; mã và kiểm thử đã sẵn sàng để học, xem lại và chạy tại máy.
