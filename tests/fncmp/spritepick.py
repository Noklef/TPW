# The pick (src/game/sprite/draw: 0x532bd0) against the original: the pointer's tile (0x7b05cc) any word or one near the map's edges, the
# GUI's mouse position any words, the picked draw record (0x8bd4fc) none, a shown script's (+0xd0) or another; the script VM's slot table
# (0x7b49f0: table, count, ready at 0x7b4a14) of up to 5 scripts, active (+0x114) or not; the nine (dx, dy) pairs at 0x81ed40 the
# game's ring about the tile (or any small ones); each cell in bounds its own chain of things (0x7cfb90 records, kinds that count or
# not, owners the script's slot or not; the chains run forwards, so they end).
# usage: spritepick.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

B = Bench([(0x532bd0, 0x532d82), (0x65958a, 0x6595a8), (0x475ef0, 0x475f40), (0x4d8300, 0x4d8328), (0x5da3c0, 0x5da3c1)])
TABLE, SCRIPTS, RECORDS, CELLS = RAM, RAM + 0x100, RAM + 0x1000, RAM + 0x100000
RING = [(-1, -1), (0, -1), (1, -1), (-1, 0), (0, 0), (1, 0), (-1, 1), (0, 1), (1, 1)]
KINDS = [1, 4, 5, 6, 7, 8, 0x12, 0, 2, 3, 9, 0x11, 0x13]


def case(r):
    regions = {}
    tile = r.choice([r.getrandbits(16), r.randrange(0x4000) + 1, 0, 1, 0x80, 0x81, 0x4000, 0x3f81, r.randrange(1, 0x4001)])
    regions[0x7b05cc] = struct.pack('<H', tile)
    regions[0xfaa540] = struct.pack('<2I', r.getrandbits(32), r.getrandbits(32))
    n = r.choice([0, 1, 2, 3, 5])
    table = [r.choice([0, SCRIPTS + 0x120 * k, SCRIPTS + 0x120 * k]) for k in range(n)]
    regions[TABLE] = struct.pack('<%dI' % (n + 1), *(table + [0]))
    scripts = bytearray(r.bytes(0x120 * max(n, 1)))
    for k in range(n):
        put(scripts, 0x120 * k + 0x114, 'I', r.choice([1, 1, 1, 0, r.getrandbits(32)]))
    regions[SCRIPTS] = scripts
    vm = bytearray(r.bytes(0x28))
    put(vm, 0, '2I', TABLE, r.choice([n, n, n, 1, 0, 0xffffffff]))
    put(vm, 0x24, 'I', r.choice([1, 1, 1, 0]))
    regions[0x7b49f0] = vm
    k = r.randrange(max(n, 1))
    picked = r.choice([0, SCRIPTS + 0x120 * k + 0xd0, SCRIPTS + 0x120 * k + 0xd0, SCRIPTS + 0x120 * k + 0xd0, SCRIPTS + 4])
    regions[0x8bd4f0] = struct.pack('<4I', r.getrandbits(32), r.getrandbits(32), r.getrandbits(32), picked)
    ring = RING if r.random() < 0.8 else [(r.randrange(-2, 3), r.randrange(-2, 3)) for _ in range(9)]
    regions[0x81ed40] = struct.pack('<18i', *(v for p in ring for v in p))
    regions[0x7cf83c] = struct.pack('<I', CELLS)
    owners = [r.choice([0, 1, 2, 3, 4, r.getrandbits(32)]) for _ in range(4)] + [k]
    records, things, used = bytearray(), [], 0
    row, column = ((tile - 1) & 0xffffffff) >> 7, (tile - 1) & 0x7f
    for dx, dy in ring:
        cx, cy = (dx + column) & 0xffffffff, (dy + row) & 0xffffffff
        if cx >= 0x80 or cy >= 0x80:
            continue
        cell = ((cy << 7) + cx + 1) & 0xffff
        chain = [used + 1 + i for i in range(r.choice([0, 1, 2, 3]))] if used < 12 else []
        used += len(chain)
        regions[CELLS + cell * 0x44 + 0x2b8] = struct.pack('<H', chain[0] if chain else 0)
        for i, t in enumerate(chain):
            rec = bytearray(r.bytes(0x10))
            rec[2] = r.choice(KINDS)
            put(rec, 0xa, 'H', chain[i + 1] if i + 1 < len(chain) else 0)
            put(rec, 0xc, 'I', r.choice(owners))
            things.append((t, RECORDS + len(records)))
            records += rec
    table = bytearray(r.bytes(0x14 * (used + 1)))
    for t, at in things:
        put(table, 0x14 * t, 'I', at)
    regions[0x7cfb90] = table
    regions[RECORDS] = bytes(records) or bytes(4)
    return Call(0x532bd0, [], regions, ret='a')


if __name__ == '__main__':
    B.main(case)
