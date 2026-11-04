# The advisor's drop in the table's ring (src/game/core/advisorscoring: 0x59f220) against the original: the park's age a recorder's answer, the ring's cursor before and after the wrap, the last two values falling or not, by more or less than the threshold, big enough to wrap.
# usage: advisorringdrop.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, echo, fill, log_region, put, strings  # noqa: E402

ELAPSED = 1
B = Bench([(0x59f220, 0x59f353), (0x519510, 0x519550), (0x67c320, 0x67c3e0), (0x5da3c0, 0x5da3c1)], hooks={0x4f8800: _recorder(ELAPSED, 1)})
SELF, POINTER, GAME1, GAME2, TABLE = RAM, RAM + 0x800, RAM + 0x900, RAM + 0xa00, RAM + 0x1000
RING = 0x216d8
MONTH = 0x1792f8648000
OPS = ['falls', 'falls', 'falls', 'falls', 'wrapped', 'wrapped', 'too old', 'negative age', 'one value', 'cursor below', 'first value', 'empty', 'size one', 'zero before', 'rises', 'level', 'small fall', 'big values', 'big values', 'at threshold']


def case(r):
    op = r.deal(OPS)
    log = log_region()
    self = bytearray(r.bytes(0x400))
    months = -r.randrange(1, 4) if op == 'negative age' else r.randrange(0, 40)
    put(self, 0x3c8, 'i', months - r.randrange(0, 2) if op == 'too old' else months + r.randrange(1, 6))
    put(self, 0x3cc, 'i', 95 if op == 'small fall' else r.choice([-5, 0, 10, 30]))
    age = (months * MONTH + r.choice([0, 1, MONTH - 1] if months >= 0 else [0, -1, 1 - MONTH])) & 0xffffffffffffffff
    strings(log, ELAPSED, 0, 1)
    echo(log, ELAPSED, 0)
    fill(log, ELAPSED, 0, struct.pack('<Q', age))
    ring = bytearray(r.bytes(0x120))
    size = 0 if op == 'empty' else 1 if op == 'size one' else r.randrange(2, 0x40)
    cursor = r.randrange(1, size) if size > 1 else r.randrange(-1, 2)
    wrapped = r.choice([0, 1, 0x80])
    if op == 'wrapped':
        cursor = r.choice([0, -1, -r.randrange(1, size + 1)])
        wrapped = r.choice([1, 0x80])
    elif op == 'one value':
        cursor, wrapped = 0, 0
    elif op == 'cursor below':
        cursor, wrapped = -r.randrange(1, 4), 0
    elif op == 'first value':
        cursor, wrapped = 0, 0
    latest, before = r.randrange(1, 500), r.randrange(500, 1000)
    if op == 'rises':
        latest, before = before, latest
    elif op == 'level':
        latest = before
    elif op == 'zero before':
        before = 0
    elif op == 'big values':
        latest, before = sorted((r.randrange(-1 << 31, 1 << 31), r.randrange(-1 << 31, 1 << 31)))
    if op in ('big values', 'at threshold') and before:   # the threshold about the drop itself: 100 - (latest * 100, wrapped to 32 bits) / before, signed and truncated
        wrapped100 = (latest * 100 + (1 << 31)) % (1 << 32) - (1 << 31)
        quotient = abs(wrapped100) // abs(before) * (1 if (wrapped100 < 0) == (before < 0) else -1)
        put(self, 0x3cc, 'i', (100 - quotient + r.choice([-1, 0, 0, 1]) + (1 << 31)) % (1 << 32) - (1 << 31))
    if size > 0:
        put(ring, 4 * (cursor if cursor >= 1 else max(cursor + size, 0)), 'i', before)
        put(ring, 4 + 4 * (cursor if cursor >= 0 else max(cursor + size, 0)), 'i', latest)
    put(ring, 0x18, '2i', cursor, size)
    ring[0x20] = wrapped
    regions = {LOG: log, SELF: self, 0x7cf6ec: struct.pack('<I', POINTER), POINTER: struct.pack('<I', GAME1), 0x7cf83c: struct.pack('<I', GAME2), GAME1: r.bytes(0x10), GAME2 + 0x1da720: bytes(4),
               0x7cfb90: struct.pack('<I', TABLE), TABLE + RING: ring}
    return Call(0x59f220, [SELF], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
