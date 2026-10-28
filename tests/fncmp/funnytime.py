# The game's calendar clock's constructor 0x4f7e80, its funny time now 0x4f8690 and spans 0x4f8730, 0x4f8800, 0x4f88b0
# (src/game/core/funnytime) against the originals, through the file time helpers (0x5fc530, 0x5fc5c0, 0x5fc660) and the CRT's 64-bit
# _allmul / _alldiv: GetLocalTime, SystemTimeToFileTime and FileTimeToSystemTime as recorders filling their outs, with results 0, 1 or
# their hash. The SYSTEMTIME SystemTimeToFileTime is handed is logged as the hash of its bytes up to the first 0, put within its year and
# month (its day of the week is the stack's on the original side). Now: rates and the world's ticks (+0x1da70c) about 0, 2^32 and
# products past 2^63; each field out null or a slot (at times the same one). The spans: of ticks about the same, and since ticks about
# the world's. Its users' ages in days (an object's 0x4dd670, a staff member's 0x505b70): dates about whole days before now. The park
# analyser's time stamps (Stamp: 0x4c7680, 0x4c76d0) of now, the analyser placed so that they land in OUT. Its day ring (DayRing: 0x4c7600):
# the days of a span of any ticks at the clock [[0x7cf6ec]] + 0x2a0 pushed, the ring's cursor about its size (0, 1, 50 or any up to 60).
# The world's constructor (World: 0x515540, src/game/world/world), which makes such a clock at +0x2a0: its whole block (0x1da748 bytes) any
# bytes, the clock's imports as Init's.
# usage: funnytime.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Init,Now,SpanOf,Elapsed,SpanSince,ObjectDays,StaffDays,Stamp,World,DayRing)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import IMPORT_TABLE as IAT, LOG, Bench, Call, RAM, fill, import_table, log_region, put, set_return  # noqa: E402

RANGES = [(0x4f7e80, 0x4f7f2d), (0x4f8690, 0x4f8730), (0x4f8730, 0x4f8769), (0x4f8800, 0x4f88a6), (0x4f88b0, 0x4f88f6), (0x4dd670, 0x4dd6c3), (0x505b70, 0x505bc6),
          (0x5fc530, 0x5fc550), (0x5fc550, 0x5fc570), (0x5fc5c0, 0x5fc65e), (0x5fc660, 0x5fc733), (0x67c320, 0x67c3ca), (0x67c7f0, 0x67c824),
          (0x4c7680, 0x4c76c2), (0x4c76d0, 0x4c7712), (0x4c7600, 0x4c7675)]
WORLD_RANGES = [(0x515540, 0x515660), (0x507800, 0x507840), (0x4cf030, 0x4cf050), (0x401000, 0x401030), (0x4d8da0, 0x4d8dd0), (0x50c1d0, 0x50c1f0),
                (0x502460, 0x502470), (0x4d0a90, 0x4d0ac0), (0x4d0ac0, 0x4d0af0), (0x51ad30, 0x51ad40),
                (0x67acf0, 0x67ada0), (0x507080, 0x5070a0)]
B = Bench(RANGES + WORLD_RANGES)
CLOCK, OUT, TICKS = RAM, RAM + 0x100, RAM + 0x400
WORLD_AT = RAM + 0x10000   # (World's world)
DAY = 864000000000   # FILETIME ticks
WORLD = TICKS - 0x1da70c   # (only its ticks are read)
CLOCK_AT, RING = RAM + 0x600, RAM + 0x800   # (DayRing: the world pointer [0x7cf6ec] points at; the ring's +0x21604-+0x216d8)
# The imports: slot, import table address, stack arguments, string arguments (mask), out arguments (mask).
IMPORTS = {'GetLocalTime': (40, 0x6fd164, 1, 0, 1), 'SystemTimeToFileTime': (41, 0x6fd150, 2, 1, 0), 'FileTimeToSystemTime': (42, 0x6fd0bc, 2, 0, 3)}
K = {name: v[0] for name, v in IMPORTS.items()}


def u32(r):
    return r.choice([0, 1, 4, 15000, 0xffffffff, 0x80000000, 0x7fffffff, r.randrange(0x10000), r.getrandbits(32)])


def clock_regions(r, regions, world_at=0x80239c):
    """The imports, the log and the world's ticks (the world at [world_at]) as Now needs them; returns the log."""
    log = log_region()
    regions[LOG] = log
    regions[IAT] = import_table(log, IMPORTS)
    regions[world_at] = bytearray(r.bytes(4))
    put(regions[world_at], 0, 'I', WORLD)
    regions[TICKS] = bytearray(r.bytes(0x10))
    put(regions[TICKS], 0, 'I', u32(r))
    fill(log, K['FileTimeToSystemTime'], 1, r.bytes(16), 0)
    if r.random() < 0.7:
        set_return(log, K['FileTimeToSystemTime'], r.choice([0, 1, r.getrandbits(32)]))
    return log


def case(r):
    op = r.deal(OPS)
    clock = bytearray(r.bytes(0x40))
    regions = {CLOCK: clock, OUT: bytearray(r.bytes(0x40))}
    log = clock_regions(r, regions)
    if op in ('Init', 'World'):
        st = bytearray(r.bytes(16))
        st[r.choice([1, 2, 3, 3, 3])] = 0   # (the string the SYSTEMTIME is hashed as ends within its year and month)
        fill(log, K['GetLocalTime'], 0, bytes(st), 1)
        fill(log, K['SystemTimeToFileTime'], 1, r.bytes(8), 0)
        if r.random() < 0.7:
            set_return(log, K['SystemTimeToFileTime'], r.choice([0, 1, 1, r.getrandbits(32)]))
        if r.random() < 0.3:   # (the high dword at or past 2^31: a failure undone)
            put(log, 0xb10 + 4, 'I', r.choice([0x80000000, 0x7fffffff, 0xffffffff]))
        if op == 'World':
            regions.update({WORLD_AT: r.bytes(0x1da748), 0x7cf56c: r.bytes(4), 0x801bbc: r.bytes(4), 0x7cdb40: r.bytes(0x18)})
            return Call(0x515540, [WORLD_AT], regions, conv='t', ret='a')
        return Call(0x4f7e80, [CLOCK], regions, conv='t', ret='a')
    put(clock, 0x1c, 'I', u32(r))
    if r.random() < 0.3:
        put(clock, 0, 'I', r.choice([0, 0xffffffff]))
        put(clock, 4, 'I', r.choice([0, 0x01bf53eb, 0x7fffffff, 0xffffffff]))
    if op == 'DayRing':
        regions[0x7cf6ec] = struct.pack('<I', CLOCK_AT)
        regions[CLOCK_AT] = struct.pack('<I', CLOCK - 0x2a0)
        ring = bytearray(r.bytes(0xd4))
        size = r.choice([50, 50, 50, 0, 1, 2, -1, r.randrange(60)])
        put(ring, 0xc8, 'i', r.choice([size - 1, size - 2, size, -1, 0, r.randrange(-1, 50), r.randrange(-1, 50)]))
        put(ring, 0xcc, 'i', size)
        regions[RING] = ring
        return Call(0x4c7600, [RING - 0x21604, u32(r)], regions, conv='t')
    if op in ('ObjectDays', 'StaffDays', 'Stamp'):   # (the world's own clock: the world at CLOCK - 0x2a0, its ticks moved to match)
        world = CLOCK - 0x2a0
        regions[0x80239c] = struct.pack('<I', world)
        regions[world + 0x1da70c] = regions.pop(TICKS)
        rate, ticks = struct.unpack_from('<I', clock, 0x1c)[0], struct.unpack_from('<I', regions[world + 0x1da70c])[0]
        if op == 'Stamp':
            return Call(r.choice([0x4c7680, 0x4c76d0]), [OUT - 0x21c30], regions, conv='t')
        product = rate * ticks
        seconds = product // 4 if product < 1 << 63 else -((2 ** 64 - product) // 4)
        now = struct.unpack_from('<Q', clock)[0] + seconds * 10000000
        age = r.choice([0, 1, DAY - 1, DAY, DAY + 1, 30 * DAY, 31 * DAY - 1, r.randrange(1000) * DAY + r.choice([-1, 0, 1]), -DAY, -DAY - 1,
                        r.getrandbits(64), r.randrange(1 << 63), DAY << 31, (DAY << 32) + DAY])
        address, date = (0x4dd670, 0x18) if op == 'ObjectDays' else (0x505b70, 0x1f0)   # (the object or staff member at OUT)
        regions[OUT] = bytearray(r.bytes(0x200))
        put(regions[OUT], date, 'Q', (now - age) % 2 ** 64)
        return Call(address, [OUT], regions, conv='t', ret='a')
    if op == 'SpanOf':
        return Call(0x4f8730, [CLOCK, OUT + 0x20, u32(r)], regions, conv='t', ret='a')
    if op == 'Elapsed':
        return Call(0x4f8800, [CLOCK, r.choice([OUT + 0x20, OUT + 0x20, CLOCK])], regions, conv='t', ret='a')
    if op == 'SpanSince':
        ticks = struct.unpack_from('<I', regions[TICKS])[0]
        return Call(0x4f88b0, [CLOCK, OUT + 0x20, r.choice([u32(r), ticks, ticks - 1 & 0xffffffff, ticks + 1 & 0xffffffff, ticks - r.randrange(1 << 16) & 0xffffffff])],
                    regions, conv='t', ret='a')
    outs = [0 if r.random() < 0.3 else OUT + 4 * r.choice([i, i, r.randrange(8)]) for i in range(8)]
    return Call(0x4f8690, [CLOCK, OUT + 0x20] + outs, regions, conv='t', ret='a')


KINDS = ['Init', 'Now', 'Now', 'SpanOf', 'Elapsed', 'SpanSince', 'ObjectDays', 'StaffDays', 'Stamp', 'World', 'DayRing']
OPS = [k for k in os.environ.get('OPS', ','.join(KINDS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
