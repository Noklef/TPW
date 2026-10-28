# A straight run of map cells (src/game/world/mapline) against the original: Map_ApplyLine (0x53aac0) with its cell operation 0x5370e0 a recorder (slot 20,
# three arguments: the emulator's hooks=, the driver's MapLine env), which logs the cell pointer (the world pointer at 0x7cf83c is never dereferenced: a
# random value) and the (arg, x step, y step) it gets, and answers a hash or, now and then, 0 (the run stops). Starts and ends are near the map (-8..135),
# sometimes the same, sometimes off by one or on a tie; the flags at 0x820ac8 / 0x820ad0 start random. The path's three lines (0x523230) at
# mapstep.py's path nudge world, the cell operation recorded the same way. The entrance lines (0x52a1a0): a thing near the map facing a quarter
# turn, kinds 1, 2 and others, the two outputs apart or the same dword.
# usage: mapline.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import mapstep  # noqa: E402
from fncmp import LOG, RAM, Bench, Call, _recorder, log_region, set_return  # noqa: E402

K = 20  # the recorder slot (driver.cpp's MapLine env)
B = Bench([(0x53aac0, 0x53ad1c), (0x4d8300, 0x4d8340), (0x523230, 0x5234ce),  # (and mapstep.py's)
           (0x4d96f0, 0x4d97b0), (0x4d97b0, 0x4d97e0), (0x4d97e0, 0x4d9920), (0x4d99c0, 0x4d99e0), (0x5da3c0, 0x5da3c1), (0x50c320, 0x50c3e0),
           (0x536300, 0x53644c), (0x522850, 0x522860), (0x4d0af0, 0x4d0b27), (0x535db0, 0x535dc3), (0x53ad20, 0x53aefc),
           (0x52a1a0, 0x52a3b0)],
          hooks={0x5370e0: _recorder(K, 3)})
ENDS = RAM


def coord(r):
    return r.choice([r.randrange(-8, 136), r.randrange(0, 128), r.randrange(0, 128)])


def lines(r):
    """0x523230 at mapstep.py's path nudge world (the cell, its parent, the entries two cells away), the cell operation recorded."""
    c = mapstep.nudge(r)
    regions = dict(c.regions)
    x, y = struct.unpack('<ii', bytes(regions.pop(c.args[0])))
    log = log_region()
    if r.random() < 0.2:
        set_return(log, K, 0)
    regions[LOG] = log
    regions[0x820ac8] = bytearray(r.bytes(0x10))
    return Call(0x523230, [x & 0xffffffff, y & 0xffffffff], regions, ret='a')


def entrance_lines(r):
    """0x52a1a0: a thing at a column and row near the map (-8..135) facing a quarter turn ([0x818c4c]), kinds 1, 2 and others, the outputs apart or the
    same dword; the cell operation recorded as the line's."""
    log = log_region()
    if r.random() < 0.2:
        set_return(log, K, 0)
    thing = bytearray(r.bytes(0x20))
    struct.pack_into('<ii', thing, 8, coord(r), coord(r))
    outs = RAM + 0x100
    world = r.getrandbits(32) | 0x80000000
    flags = struct.pack('<IIII', r.getrandbits(32), r.getrandbits(32), r.getrandbits(32), r.getrandbits(32))
    regions = {RAM + 0x200: thing, outs: r.bytes(8), LOG: log, 0x7cf83c: struct.pack('<I', world), 0x820ac8: flags,
               0x818c4c: struct.pack('<i', r.choice([0, 0x5a, 0xb4, 0x10e])), 0x818c50: r.bytes(4), 0x7639e0: r.bytes(8), 0x8186cc: r.bytes(4)}
    kind = r.choice([1, 2, 0, 3, 0xd, 0x80, r.getrandbits(32)])
    return Call(0x52a1a0, [RAM + 0x200, outs, outs if r.random() < 0.2 else outs + 4, kind], regions, ret='a')


def case(r):
    op = r.deal(OPS or [0x53aac0, 0x523230, 0x52a1a0])
    if op == 0x523230:
        return lines(r)
    if op == 0x52a1a0:
        return entrance_lines(r)
    x, y = coord(r), coord(r)
    shape = r.choice(['any', 'any', 'h', 'v', 'tie', 'same', 'near'])
    if shape == 'h':
        ex, ey = coord(r), y + r.randrange(-3, 4)
    elif shape == 'v':
        ex, ey = x + r.randrange(-3, 4), coord(r)
    elif shape == 'tie':
        d = r.randrange(-40, 41)
        ex, ey = x + d, y + r.choice([d, -d])
    elif shape == 'same':
        ex, ey = x, y
    elif shape == 'near':
        ex, ey = x + r.randrange(-2, 3), y + r.randrange(-2, 3)
    else:
        ex, ey = coord(r), coord(r)
    log = log_region()
    if r.random() < 0.2:
        set_return(log, K, 0)
    world = r.getrandbits(32) | 0x80000000
    flags = struct.pack('<IIII', r.getrandbits(32), r.getrandbits(32), r.getrandbits(32), r.getrandbits(32))
    return Call(0x53aac0, [r.getrandbits(32), x & 0xffffffff, y & 0xffffffff, ENDS, ENDS + 4],
                {ENDS: bytearray(struct.pack('<ii', ex, ey)), LOG: log, 0x7cf83c: bytearray(struct.pack('<I', world)), 0x820ac8: bytearray(flags)},
                ret='a')


OPS = [int(k, 16) for k in (os.environ.get('OPS', '') if __name__ == '__main__' else '').split(',') if k]


if __name__ == '__main__':
    B.main(case)
