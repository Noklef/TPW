"""tools/ranges.py ADDR... [--depth N] [--max N] [--margin HEX]: the code ranges a tests/fncmp bench needs to run the originals of ADDR... in the emulator.

Follows the direct calls of each function through the Ghidra listing (_re/text.asm) and prints a Bench([...]) range list. Calls the bench harness answers
itself (the heap, the STL locks, the runtime helpers, the debug print) are not followed. Indirect calls (CALL reg / [mem]) cannot be followed: they are
counted per function, and a bench must give them recorders (fncmp.vtable) or hooks. Test tooling only: nothing here is game code.

A range that is too small makes the bench stop with "no listing for ADDR"; one that is too big does no harm. So the output is a starting point, not a proof.
"""
import bisect
import os
import re
import sys

LIST = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '_re', 'text.asm')
# answered by the harness (tools/fncmp.py): not followed. The debug print gets the one-byte range the benches give it.
HARNESS = {0x67acc0: 'Mem_Alloc', 0x67acb0: 'Mem_Free', 0x691de0: 'STL lock', 0x691ea0: 'STL unlock', 0x67a830: '_ftol', 0x67b280: 'modf', 0x67b24a: '_CIfmod'}
PRINT = 0x5da3c0


def load():
    addrs, lines = [], []
    for ln in open(LIST):
        m = re.match(r'([0-9a-f]{8})  (.*)', ln)
        if m:
            addrs.append(int(m.group(1), 16))
            lines.append(m.group(2))
    return addrs, lines


def extent(addrs, lines, start):
    """(end, direct call targets, indirect call count) of the function at start: its first RET or JMP past every forward jump target."""
    j = bisect.bisect_left(addrs, start)
    far, calls, indirect = start, [], 0
    while j < len(addrs):
        t = lines[j]
        m = re.search(r'J\w+ 0x([0-9a-f]{8})', t)
        if m and not t.startswith('JMP dword'):
            tgt = int(m.group(1), 16)
            if tgt > far and tgt - start < 0x1000:
                far = tgt
        m = re.match(r'CALL 0x([0-9a-f]{8})', t)
        if m:
            calls.append(int(m.group(1), 16))
        elif t.startswith('CALL'):
            indirect += 1
        nxt = addrs[j + 1] if j + 1 < len(addrs) else addrs[j] + 1
        if (t.startswith('RET') or t.startswith('JMP')) and nxt > far:
            return nxt, calls, indirect
        j += 1
    return addrs[-1] + 1, calls, indirect


def main(argv):
    depth, limit, margin = 99, 40, 0x10
    roots = []
    i = 0
    while i < len(argv):
        if argv[i] == '--depth':
            depth = int(argv[i + 1]); i += 2
        elif argv[i] == '--max':
            limit = int(argv[i + 1]); i += 2
        elif argv[i] == '--margin':
            margin = int(argv[i + 1], 16); i += 2
        else:
            roots.append(int(argv[i], 16)); i += 1
    addrs, lines = load()
    seen, order, indirect_total = {}, [], 0
    todo = [(r, 0) for r in roots]
    while todo:
        a, d = todo.pop(0)
        if a in seen or a in HARNESS or a == PRINT:
            continue
        if len(seen) >= limit:
            print('stopped following at %d functions (--max); %x and the rest are not in the list' % (limit, a))
            break
        if bisect.bisect_left(addrs, a) >= len(addrs) or addrs[bisect.bisect_left(addrs, a)] != a:
            print('%x: not an instruction start in the listing (an import, or data); skipped' % a)
            seen[a] = None
            continue
        end, calls, indirect = extent(addrs, lines, a)
        seen[a] = (end, calls, indirect, d)
        order.append(a)
        indirect_total += indirect
        if d < depth:
            todo.extend((c, d + 1) for c in calls)
    ranges = []
    for a in sorted(x for x in seen if seen[x]):
        end = seen[a][0] + margin
        if ranges and a <= ranges[-1][1]:
            ranges[-1][1] = max(ranges[-1][1], end)
        else:
            ranges.append([a, end])
    for a in order:
        end, calls, indirect, d = seen[a]
        flags = []
        if indirect:
            flags.append('%d indirect call%s' % (indirect, '' if indirect == 1 else 's'))
        if end - a > 0x400:
            flags.append('big')
        print('  %x size %#x depth %d%s' % (a, end - a, d, ('  [' + ', '.join(flags) + ']') if flags else ''))
    used = sorted({c for a in order for c in seen[a][1] if c in HARNESS})
    if used:
        print('harness-answered calls: ' + ', '.join('%x %s' % (c, HARNESS[c]) for c in used))
    print('Bench([%s%s])' % (', '.join('(%#x, %#x)' % tuple(r) for r in ranges), (', (%#x, %#x)' % (PRINT, PRINT + 1)) if any(PRINT in seen[a][1] for a in order) else ''))
    if indirect_total:
        print('%d indirect call(s) in all: give them recorders or hooks in the bench' % indirect_total)


if __name__ == '__main__':
    main(sys.argv[1:])
