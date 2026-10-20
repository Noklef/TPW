r"""Code the listing never decoded because only an instruction's immediate points at it (MOV [reg+n],imm / PUSH imm: method pointers
stored at run time, atexit destructors, catch funclets' continuation addresses). tools/FlowGaps.java covers CALL / Jcc / JMP targets.

  py -I tools/immrefs.py [LISTING]   (default _re/text.asm)

Prints each immediate into .text with no instruction at it, its first bytes, the listed instruction before it and who refers to it.
Targets on padding (0x90 / 0xcc) are skipped. A target whose preceding line is a RET or JMP and whose first byte is not 00 is marked
"*": a function start, or a continuation when the referrer is just before it (`MOV EAX,cont; RET` ends a catch block). The rest are
constants that only look like addresses (switch byte tables, colours, sizes).
"""
import bisect
import os
import re
import struct
import sys

ROOT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))
EXE = os.path.join(ROOT, '.data', 'nocd', 'tp.exe')
LO, HI = 0x401000, 0x6e6a7a


def main():
    listing = sys.argv[1] if len(sys.argv) > 1 else os.path.join(ROOT, '_re', 'text.asm')
    addrs, text, refs = [], {}, {}
    for line in open(listing):
        line = line.rstrip('\n')
        if not line:
            continue
        a = int(line[:8], 16)
        addrs.append(a)
        text[a] = line[10:]
        if 'CALL' in line or line[10:].startswith('J'):
            continue
        for m in re.finditer(r'0x0*([0-9a-f]{6})\b', line[10:]):
            t = int(m.group(1), 16)
            if LO <= t < HI:
                refs.setdefault(t, []).append(a)
    data = open(EXE, 'rb').read()
    pe = struct.unpack_from('<I', data, 0x3c)[0]
    opt = struct.unpack_from('<H', data, pe + 20)[0]
    base = struct.unpack_from('<I', data, pe + 52)[0]
    secs = []
    for i in range(struct.unpack_from('<H', data, pe + 6)[0]):
        vsize, va, rsize, raw = struct.unpack_from('<IIII', data, pe + 24 + opt + 40 * i + 8)
        secs.append((base + va, max(vsize, rsize), raw))

    def at(a, n):
        for va, size, raw in secs:
            if va <= a < va + size:
                return data[raw + a - va:raw + a - va + n]
        return b''

    n = 0
    for t in sorted(refs):
        if t in text:
            continue
        b = at(t, 6)
        if not b or b[0] in (0x90, 0xcc):
            continue
        i = bisect.bisect_right(addrs, t) - 1
        prev = addrs[i] if i >= 0 else 0
        mark = '*' if prev and re.match(r'(RET|JMP)\b', text[prev]) and b[0] != 0 else ' '
        n += 1
        print('%s %06x  %s  after %06x %-28s  from %s' % (mark, t, b.hex(' '), prev, text.get(prev, '')[:28],
                                                    ' '.join('%06x' % x for x in refs[t][:3])))
    print(n, 'targets', file=sys.stderr)


if __name__ == '__main__':
    main()
