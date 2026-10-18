# The GUI library's timed messages (src/game/ui/guitimers: 0x669c62, 0x669c8d, 0x669d77, 0x669e3a, 0x669eb3, 0x669efb) against the
# originals: a table of 1-6 entries (free or not, due around the clock's time or anywhere, their windows among two with a logging
# handler or not, and 0), the entries now and then missing; the clock [0x77c490] a recorder vtable whose slot 0 answers a time near 0,
# the sign bit or the top, or random. Init: the count 0-6, 0x1ff, 0x200 or -1, the allocation failing now and then. Add: the delay 0,
# 1, 59999, 60000, below or anywhere.
# usage: guitimers.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Init,Free,Add,Fire,Cancel,Reset)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import CDECL_LOG4, HEAP, LOG, RAM, Bench, Call, heap, log_region, put, set_return, vtable  # noqa: E402

CALLS = {'Init': 0x669efb, 'Free': 0x669c62, 'Add': 0x669c8d, 'Fire': 0x669d77, 'Cancel': 0x669e3a, 'Reset': 0x669eb3}
B = Bench([(0x669c62, 0x669c8d), (0x669c8d, 0x669d77), (0x669d77, 0x669e3a), (0x669e3a, 0x669eb3), (0x669eb3, 0x669efb),
           (0x669efb, 0x669f79), (0x65968e, 0x6596a2), (0x65f697, 0x65f6d1)])
TABLE, ENTRIES, WINDOWS, CLOCK, VTABLE = RAM, RAM + 0x100, RAM + 0x400, RAM + 0x800, RAM + 0x900
WINS = [WINDOWS, WINDOWS + 0x200]


def case(r):
    op = r.deal(OPS)
    log = log_region()
    now = r.choice([1000, 0x7ffffff0, 0xfffffff0, r.getrandbits(32)])
    set_return(log, 0, now)
    regions = {LOG: log, VTABLE: vtable({}), CLOCK: struct.pack('<I', VTABLE), 0x77c490: struct.pack('<I', CLOCK)}
    for w in WINS:
        win = bytearray(r.bytes(0x120))
        put(win, 0x114, 'I', r.choice([CDECL_LOG4] * 3 + [0]))
        regions[w] = win
    n = r.randrange(1, 7)
    entries = bytearray(r.bytes(0x14 * n))
    for i in range(n):
        due = r.choice([0, 0, (now + r.randrange(-5, 6)) & 0xffffffff, (now + r.randrange(-5, 6)) & 0xffffffff, r.getrandbits(32)])
        put(entries, 0x14 * i, 'I', r.choice(WINS + [0, r.getrandbits(32)]) if due == 0 else r.choice(WINS * 6 + [0]))
        put(entries, 0x14 * i + 0x10, 'I', due)
    regions[ENTRIES] = entries
    regions[TABLE] = struct.pack('<iI', r.choice([n, n, n, n - 1, 0]), r.choice([ENTRIES] * 9 + [0]))
    if op == 'Init':
        regions[HEAP] = heap(0x3000, r.choice([0] * 6 + [1]))
        regions[TABLE] = struct.pack('<II', r.getrandbits(32), r.choice([0] * 6 + [ENTRIES]))
        count = r.choice([0, 1, 3, 6, 6, 0x1ff, 0x200, -1, r.getrandbits(32)])
        return Call(CALLS[op], [TABLE, count & 0xffffffff], regions, conv='t', ret='w')
    if op == 'Free':
        regions[HEAP] = heap(0x40)
        regions[TABLE] = struct.pack('<iI', n, r.choice([HEAP + 0x10, HEAP + 0x10, 0]))
        return Call(CALLS[op], [TABLE], regions, conv='t')
    if op == 'Add':
        delay = r.choice([0, 1, 59999, 60000, r.randrange(1, 60000), r.randrange(1, 60000), r.getrandbits(32)])
        args = [r.choice(WINS + [r.getrandbits(32)])] + [r.getrandbits(32) for _ in range(3)] + [delay]
        return Call(CALLS[op], [TABLE] + args, regions, conv='t', ret='w')
    if op == 'Cancel':
        return Call(CALLS[op], [TABLE, r.choice(WINS + [0])], regions, conv='t', ret='w')
    return Call(CALLS[op], [TABLE], regions, conv='t', ret='w' if op == 'Fire' else 'v')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
