# OS Assignment 1 - xv6

CS-33211: System Calls and User Space in xv6. Based on the full history of [guanslab/xv6-a1-starter](https://github.com/guanslab/xv6-a1-starter).

## Build and run

Install a RISC-V GCC/binutils toolchain and QEMU 7.2 or newer, then:

```sh
make clean
make qemu
```

In the xv6 shell:

```text
sleep 2
find . *.c -type f
echo a b c d e | xargs -n 2 echo x
trace 32800 grep hello README
ps
a1test
usertests -q
```

Quit QEMU with Ctrl-a followed by x. To run the host-side assignment checks on a fresh filesystem:

```sh
make clean
make kernel/kernel fs.img
python3 tests/functional.py
```

The Oct 10 Canvas announcement supersedes the handout's `make grade` instruction: the supplied starter has no `grade-a1`, and the TA will review and grade the code manually.

## Implementation notes

- `sleep`: strict decimal parsing, overflow rejection, `pause(ticks)`, and required stderr usage text.
- `find`: literal matching plus `*`, directory-order traversal, file/directory filtering, and exclusion of the starting directory and `.`/`..`. Depth-first traversal uses explicit heap frames; parent descriptors close before descent, avoiding call-stack growth.
- `xargs`: whitespace tokenization; nonempty-line execution by default; `-n` batching across line boundaries; sequential fork/exec/wait; accumulated child failures; dynamically sized words.
- `trace` (23): process-local mask inherited by `kfork`, retained by exec, and cleared on allocation/free. Save `a0` and path arguments before the handler; inspect the mask afterwards. Each trace line is emitted in one `printk` call.
- `sysinfo` (24): free-list pages under `kmem.lock`; non-UNUSED processes under their locks; live file-table entries under `ftable.lock`. Results cross to user memory with `copyout`.
- `getprocs` (25): `wait_lock` before each process lock, one zero-initialized stack record at a time, and no locks held during `copyout`. Check negative limits, bad destinations, and address overflow.
- `ps`: tab-separated fields and the prescribed state names, with `%lu` for size.
- `MAXARG` means 32 actual arguments. The starter's exec arrays also needed an extra slot for the terminating null; boundary behavior is tested.
- Modern linkers warn about the starter's intentional RWX ELF segments. The Makefile suppresses that specific inherited diagnostic when supported; `-Wall -Werror` remains enabled.

`a1test` checks argument errors, exit statuses, 52-level directory traversal, wildcard placement, batching, 32 arguments, long words, memory/file/process counts, invalid and lazy destinations, parent PIDs, and concurrent fork/exit. The host runner checks trace output and `ps` formatting. The full xv6 regression suite is separate (`usertests -q`). Passing these checks is evidence of tested behavior, not a claim about hidden instructor tests.

## Giải thích ngắn bằng tiếng Việt

- `sleep` kiểm tra từng chữ số rồi gọi `pause`; `atoi` đơn thuần không đủ để phát hiện input sai.
- `find` đọc danh sách tên, đóng thư mục cha rồi mới đi xuống. Cách này giảm nguy cơ hết file descriptor hoặc tràn stack khi cây sâu.
- `xargs` gom từ, tạo child chạy lệnh và đợi child xong trước lượt tiếp theo. `-n` bỏ qua ranh giới dòng; không có `-n` thì mỗi dòng không rỗng là một lượt.
- Khi system call đi qua `ecall` → `usertrap` → `syscall`, thanh ghi `a7` chọn handler và `a0` chứa đối số/giá trị trả về. Trace phải lưu đối số trước khi gọi handler; exec thành công còn thay cả vùng nhớ chứa tên file.
- `sysinfo` đọc từng cấu trúc dưới đúng khóa. `getprocs` luôn lấy `wait_lock` trước `p->lock`, chụp một record vào stack, nhả khóa rồi mới `copyout`.
- Bài được phép dùng AI; phần cài đặt này được hỗ trợ bởi AI và đã được kiểm tra thực tế. Người nộp vẫn cần hiểu và chịu trách nhiệm về mã.

## References and submission

- [Assignment on Canvas](https://kent.instructure.com/courses/137884/assignments/2124766)
- [Deadline / grading update](https://kent.instructure.com/courses/137884/discussion_topics/1243802)
- [MIT xv6 book](https://pdos.csail.mit.edu/6.1810/2025/xv6/book-riscv-rev5.pdf), particularly process/file-descriptor interfaces, memory allocation, and syscall arguments.

Submission is the repository URL in Canvas, with the final commit tagged `a1-submit`. `time.txt` must contain the student's actual hours as one integer; it must be supplied accurately before tagging the final submission.

The original MIT README and license are retained.
