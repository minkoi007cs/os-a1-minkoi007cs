import os,re,select,signal,subprocess,time
from pathlib import Path
ROOT=Path(__file__).resolve().parent.parent
LOG=Path(__file__).resolve().parent/'functional.log'
proc=subprocess.Popen(['make','qemu'],cwd=ROOT,stdin=subprocess.PIPE,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,start_new_session=True)
all_output=bytearray()
def read_until(pattern,timeout=120):
 block=bytearray(); end=time.monotonic()+timeout
 while time.monotonic()<end:
  if select.select([proc.stdout],[],[],0.2)[0]:
   data=os.read(proc.stdout.fileno(),65536)
   if not data:raise RuntimeError('QEMU exited')
   block.extend(data);all_output.extend(data)
   text=block.decode(errors='replace').replace('\r','')
   if 'panic:' in text or 'A1 FAIL:' in text:raise RuntimeError(text[-2000:])
   if re.search(pattern,text):return text
 raise RuntimeError('timeout: '+block.decode(errors='replace')[-2500:])
def command(text):
 proc.stdin.write((text+'\n').encode());proc.stdin.flush()
 return read_until(r'\$ ')
def expect(ok,description):
 if not ok:raise AssertionError(description)
 print('PASS:',description,flush=True)
try:
 read_until(r'\$ ',30)
 out=command('a1test')
 expect('A1 TESTS PASSED' in out,'assignment utilities + kernel assertions')
 print(re.search(r'A1 TESTS PASSED.*',out).group(),flush=True)
 out=command('ps')
 expect('PID\tPPID\tSTATE\tSIZE\tNAME' in out,'ps exact tab-separated header')
 rows=re.findall(r'^\d+\t\d+\t(?:used|sleeping|runnable|running|zombie)\t\d+\t\S+',out,re.M)
 expect(len(rows)>=3,'ps process rows and state names')
 out=command('a1test trace')
 expect(re.search(r'\[\d+\] trace\(\d+\) -> 0',out),'trace traces itself after updating mask')
 expect(re.search(r'\[\d+\] kill\(-123\) -> -1',out),'trace signed argument and return')
 expect(re.search(r'\[\d+\] open\(\?\) -> -1',out),'trace inaccessible string pointer')
 pids=re.findall(r'\[(\d+)\] getpid\(\) -> (\d+)',out)
 expect(len({p for p,r in pids if p==r})>=2,'trace mask inherited across fork')
 expect(re.search(r'\[\d+\] exec\("echo"\) -> 2',out) and 'TRACE_EXEC_OK' in out,'trace saves path before successful exec')
 expect(not re.search(r'\[\d+\] exit\(',out),'nonreturning exit is not traced')
 out=command('trace 128 echo WRAPPER_EXEC_OK')
 expect(re.search(r'\[\d+\] exec\("echo"\) -> 2',out),'trace wrapper and mask across exec')
 out=command('trace 8388608 echo SELF_MASK_OK')
 expect(re.search(r'\[\d+\] trace\(8388608\) -> 0',out),'trace syscall number 23')
 out=command('trace 8192 sleep 0')
 expect(re.search(r'\[\d+\] pause\(0\) -> 0',out),'numeric a0 captured before return')
 out=command('trace 32 cat README')
 expect(re.search(r'\[\d+\] read\(3\) -> [1-9]\d*',out),'read trace prints fd rather than overwritten return value')
 out=command('echo TRACE_CLEAN')
 expect('TRACE_CLEAN' in out and not re.search(r'\[\d+\]',out),'freed process mask does not leak into reused slot')
 print('ALL A1 FUNCTIONAL TESTS PASSED',flush=True)
finally:
 LOG.write_bytes(all_output)
 if proc.poll() is None:
  proc.stdin.write(b'\x01x');proc.stdin.flush()
  try:proc.wait(timeout=5)
  except subprocess.TimeoutExpired:os.killpg(proc.pid,signal.SIGTERM)
