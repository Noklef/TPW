"""tools/extent.py ADDR...: the end of each function (first RET / JMP past every forward jump target) from the Ghidra listing"""
import bisect
import re
import sys

import os
LIST = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '_re', 'text.asm')
addrs, lines = [], []
for ln in open(LIST):
    m = re.match(r'([0-9a-f]{8})  (.*)', ln)
    if m:
        addrs.append(int(m.group(1), 16))
        lines.append(m.group(2))
for a in sys.argv[1:]:
    start = int(a, 16)
    i = bisect.bisect_left(addrs, start)
    far = start
    end = start
    j = i
    while j < len(addrs):
        t = lines[j]
        m = re.search(r'J\w+ 0x([0-9a-f]{8})', t)
        if m and not t.startswith('JMP dword'):
            tgt = int(m.group(1), 16)
            if tgt > far and tgt - start < 0x1000:
                far = tgt
        nxt = addrs[j + 1] if j + 1 < len(addrs) else addrs[j] + 1
        if (t.startswith('RET') or t.startswith('JMP')) and nxt > far:
            end = nxt
            break
        j += 1
    print('%x size %#x (round %#x)' % (start, end - start, ((end - start) + 15) & ~15))
