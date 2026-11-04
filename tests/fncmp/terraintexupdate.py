# The terrain tile cache's least-recently-used update (src/game/terrain/texcache: 0x572440) against the original: 0-8 tiles asked for, from a small set of keys, some already held; a ring of 1-8 cache tiles, with slots or none; each record's texture or none and flag 0x100; the blend, locks and cache calls recorders; the format's pixel size 2 or 4, the side any small one.
# usage: terraintexupdate.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, log_fold, log_region, put, set_return  # noqa: E402

FLIPPED, LOCK, BLEND, CONVERT, UNLINK, KEEP, TOUCH = 1, 2, 3, 4, 5, 6, 7
B = Bench([(0x572440, 0x57262b)], hooks={0x590da0: _recorder(FLIPPED, 0), 0x590d40: _recorder(LOCK, 0), 0x55f780: _recorder(BLEND, 4), 0x562f70: _recorder(CONVERT, 3, cdecl=True),
                                         0x58dc50: _recorder(UNLINK, 1), 0x590f90: _recorder(KEEP, 0), 0x58c9f0: _recorder(TOUCH, 2)})
CACHE, NODES, TILES, RECORDS, PIXELS = RAM, RAM + 0x20000, RAM + 0x21000, RAM + 0x21100, RAM + 0x22000


def index(key):
    return ((key >> 8) << 7) + (key & 0xff)


def case(r):
    log = log_region()
    log_fold(log)
    keys = [r.randrange(0x7f) << 8 | r.randrange(0x100) for _ in range(6)]
    cache = bytearray(0x10014)
    side = r.choice([0, 1, 2, 4, 8])
    put(cache, 0, '2I', r.getrandbits(32), side)
    n = r.randrange(1, 9)
    nodes = bytearray(0x10 * n)
    held = list(dict.fromkeys(r.choice(keys) for _ in range(r.randrange(n + 1))))
    held = [k for i, k in enumerate(held) if index(k) not in [index(o) for o in held[:i]]]
    for i in range(n):
        slot = 0
        if i < len(held):
            slot = CACHE + 8 + 4 * index(held[i])
            put(cache, 8 + 4 * index(held[i]), 'I', NODES + 0x10 * i)
        put(nodes, 0x10 * i, '4I', NODES + 0x10 * ((i - 1) % n), NODES + 0x10 * ((i + 1) % n), slot, r.getrandbits(32))
    put(cache, 0x1000c, 'I', NODES + 0x10 * r.randrange(n))
    count = r.choice([0, 1, 2, 3, 4, 8, -1])
    tiles = b''.join(struct.pack('<I', r.choice(keys)) for _ in range(8))
    records = b''.join(struct.pack('<I', r.choice([0, r.getrandbits(32)])) + r.bytes(0xc) + struct.pack('<I', r.getrandbits(32) & ~0x100 | r.choice([0, 0x100])) for _ in range(8))
    set_return(log, FLIPPED, r.getrandbits(32))
    set_return(log, LOCK, PIXELS)
    regions = {LOG: log, CACHE: cache, NODES: nodes, TILES: tiles, RECORDS: records, PIXELS: r.bytes(0x100), 0x8bd554: struct.pack('<I', r.choice([0, 1, 2, 3, 3, 3, 4])),
               0x879a68: r.bytes(4), 0x8c4b44: r.bytes(4)}
    return Call(0x572440, [CACHE, TILES, RECORDS, count], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
