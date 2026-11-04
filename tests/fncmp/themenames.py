# A theme record's names as text (src/game/core/themenames: 0x5b12e0, 0x5b1410, 0x5b1520) against the originals: snwprintf, swprintf and the object definitions' three names recorders (what they are given is
# compared; the statics they would fill stay as they are). The names empty, short, ending in one or two dashes; the name changed or not, the global.sam there or not (the heap failing its block).
# usage: themenames.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=FullName,Name,Suffix)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, Bench, Call, Mask, _recorder, heap, log_region, put, strings  # noqa: E402

SNW, SW, FULL, NAME, SUFFIX = 1, 2, 3, 4, 5
B = Bench([(0x5b12e0, 0x5b140f), (0x5b1410, 0x5b1515), (0x5b1520, 0x5b1594), (0x5b11c0, 0x5b12d3), (0x409cd0, 0x409cdc), (0x67acd0, 0x67acee), (0x67b0d0, 0x67b10f)],
          hooks={0x67c830: _recorder(SNW, 5, cdecl=True), 0x67af00: _recorder(SW, 2, cdecl=True), 0x412fb0: _recorder(FULL, 3), 0x413020: _recorder(NAME, 3), 0x4130b0: _recorder(SUFFIX, 3)})
OPS = {k: a for k, a in (('FullName', 0x5b12e0), ('Name', 0x5b1410), ('Suffix', 0x5b1520)) if not os.environ.get('OPS') or k in os.environ['OPS'].split(',')}
RECORD, TO, SAM = RAM, RAM + 0x1000, RAM + 0x2000


def units(r):
    n = r.choice([0, 0, 1, 2, 3, 7, 0x1f])
    tail = r.choice([[], [0x2d], [0x2d], [0x2d, 0x2d], [0x2d, 0x2d], [0x41, 0x2d], [0x2d, 0x41]])[:n]
    return [r.choice([0x41, 0x62, 0x20, 0x2d]) for _ in range(n - len(tail))] + tail


def case(r):
    op = r.deal(list(OPS))
    log = log_region()
    a, b = units(r), units(r)
    record = bytearray(r.bytes(0xbc))
    struct.pack_into('<%dH' % (len(a) + 1), record, 0x34, *a, 0)
    struct.pack_into('<%dH' % (len(b) + 1), record, 0x76, *b, 0)
    changed = r.random() < 0.75
    record[0xb8] = r.choice([1, 0x80, r.randrange(1, 256)]) if changed else 0
    sam = r.random() < 0.7
    put(record, 0x1c, 'I', SAM if sam else 0)
    # (the recorder takes five dwords: those a call does not pass are the stack's)
    if not changed or op == 'Suffix':
        hide = 0b11000 if op != 'Suffix' or not changed else 0b10000
    elif op == 'Name':
        hide = 0b11000 if not a else 0b10000
    else:
        hide = 0b10000 if a and a[-1] == 0x2d else 0
    strings(log, SNW, 0, out=hide)
    regions = {LOG: log, RECORD: record, TO: r.bytes(0x60), SAM: r.bytes(0x20), 0xf7bf70: r.bytes(0x60), 0xf7c7f0: r.bytes(0x60), HEAP: heap(fail=0 if sam else 1)}
    # (the id's argument: 0x409cd0 writes its low word, the high half is the stack's; the callees use the low word)
    masks = [Mask(LOG, [0x12, 0x13], "the id argument's high half, a stack leftover")] if sam and not changed else []
    return Call(OPS[op], [RECORD, TO, r.choice([0, 1, 0x10, 0x21, r.randrange(0x40)])], regions, conv='t', ret='v', masks=masks)


if __name__ == '__main__':
    B.main(case)
