# The 64-bit time (src/game/core/filetime: 0x5fc530-0x5fc730, 0x5fcc50) against the originals: sums, differences, compares and copies of
# times about the carries and signs (the result where an operand is, or apart); FromSystemTime with fields of any high bits, the import
# (SystemTimeToFileTime, a recorder: the SYSTEMTIME on the stack hashed but for its day of the week, which the code leaves as it finds it)
# failing or writing a time about 2^63; ToSystemTime with the import failing or filling the SYSTEMTIME, each out pointer set or null;
# FromSpan with days to milliseconds about the 32-bit wrap and the range check's bounds.
# usage: filetime.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Add,... to pick)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import IMPORT_TABLE, LOG, RAM, Bench, Call, blob_bytes, fill, import_table, log_region, set_return  # noqa: E402

B = Bench([(0x5fc530, 0x5fc750), (0x5fcc50, 0x5fccf0), (0x67c7f0, 0x67c840)])
CALLS = {'Add': 0x5fc530, 'Sub': 0x5fc550, 'Greater': 0x5fc570, 'Assign': 0x5fc5a0, 'FromSystemTime': 0x5fc5c0, 'ToSystemTime': 0x5fc660,
         'FromSpan': 0x5fcc50}
# name: (recorder slot, import slot, stack arguments, string arguments, out arguments)
IMPORTS = {'SystemTimeToFileTime': (41, 0x6fd150, 2, 0, 0), 'FileTimeToSystemTime': (42, 0x6fd0bc, 2, 0, 2)}
T, OUT, B2, SINK = RAM, RAM + 0x10, RAM + 0x20, RAM + 0x40


def time(r):
    high = r.choice([0, 1, 0x7fffffff, 0x80000000, 0xffffffff, 0x01d00000, r.getrandbits(32)])
    low = r.choice([0, 1, 0x7fffffff, 0x80000000, 0xffffffff, r.getrandbits(32)])
    return struct.pack('<II', low, high)


def field(r):
    return r.choice([0, 1, 12, 31, 59, 999, 1601, 1999, 30827, 0xffff, 0x10000, r.randrange(0x10000), r.getrandbits(32)])


def span(r):
    return r.choice([0, 1, -1, 24, 60, 1000, 10675199, -10675199, 0x7fffffff, -0x80000000, r.randrange(-100000, 100000), r.getrandbits(32) - (1 << 31)])


def case(r):
    op = r.deal(OPS)
    mem = bytearray(r.bytes(0x100))
    mem[0:8] = time(r)
    mem[0x20:0x28] = time(r)
    if r.random() < 0.3:   # (equal highs: the low halves decide)
        mem[0x24:0x28] = mem[4:8]
    regions = {RAM: mem}
    if op in ('Add', 'Sub'):
        out, b = r.choice([OUT, OUT, T, B2]), r.choice([B2, B2, T])
        return Call(CALLS[op], [T, out, b], regions, conv='t', ret='a')
    if op == 'Greater':
        return Call(CALLS[op], [T, r.choice([B2, B2, B2, T])], regions, conv='t', ret='a')
    if op == 'Assign':
        return Call(CALLS[op], [T, r.choice([B2, B2, T])], regions, conv='t', ret='a')
    if op == 'FromSpan':
        return Call(CALLS[op], [T, SINK] + [span(r) & 0xffffffff for _ in range(5)], regions, conv='t', ret='a')
    log = log_region()
    regions.update({LOG: log, IMPORT_TABLE: import_table(log, IMPORTS)})
    if op == 'FromSystemTime':
        k = IMPORTS['SystemTimeToFileTime'][0]
        blob_bytes(log, k, 0, 0xffcf)   # (all but wDayOfWeek)
        set_return(log, k, r.choice([0, 1, 1, 1, r.getrandbits(32)]))
        if r.random() < 0.8:
            fill(log, k, 1, time(r))
        return Call(CALLS[op], [T, SINK] + [field(r) for _ in range(7)], regions, conv='t', ret='a')
    k = IMPORTS['FileTimeToSystemTime'][0]
    set_return(log, k, r.choice([0, 1, 1, 1, r.getrandbits(32)]))
    fill(log, k, 1, r.bytes(16))
    return Call(CALLS[op], [T, SINK] + [r.choice([0, SINK + 8 + 4 * i]) for i in range(8)], regions, conv='t', ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
