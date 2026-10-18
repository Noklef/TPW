# The window at 0x7cb398 (src/game/ui/dialog7cb398: 0x4a4ab0) against the original. ShowTopThree: thingcount.py's park (thingenv.py) with values
# 0-22 at +0x30 (the game record's handle +0x1da720 of [0x7cf83c] one of the slots); the window's children (a few others with them, the child
# 0x4a68d1 now and then missing) and that child's 0x4a68d2..0x4a68d4 (their slot 7 a recorder, their own children none or window.py's tree
# for Window_SetShown, shown_env).
# usage: dialog7cb398.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=ShowTopThree)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import LOG, RAM, Bench, Call, log_region, put, vtable  # noqa: E402
from thingenv import THING, park  # noqa: E402
from window import RANGES as WINDOW_RANGES, node, shown_env  # noqa: E402

CALLS = {'ShowTopThree': 0x4a4ab0}
B = Bench([(0x4a4ab0, 0x4a4b8a), (0x4c8430, 0x4c85a8), (0x516130, 0x516160), (0x516160, 0x5161b0), (0x50b350, 0x50b360), (0x5da3c0, 0x5da3c1),
           (0x4fa990, 0x4fa9fa), (0x536300, 0x53644c), (0x4d8300, 0x4d8328), (0x519510, 0x519550)] + WINDOW_RANGES)
ROOT, PANEL, KIDS, KVTABLE, DECOYS, POOL = 0x33000000, 0x33010000, 0x33020000, 0x33030000, 0x33040000, 0x34000000   # (window.py's tree is at RAM + 0x3000)
NODE = 0x120


def window(r, regions, at, parent, children):
    buf = bytearray(r.bytes(NODE))
    put(buf, 0, 'I', KVTABLE)
    buf[0x48] = (buf[0x48] & 0xfe) | r.choice([0, 1, 1])
    put(buf, 0x54, 'I', 0)
    put(buf, 0x5c, 'I', children)
    put(buf, 0x60, 'I', parent)
    for off in (0x98, 0x9c, 0x114):
        put(buf, off, 'I', 0)
    regions[at] = buf
    return buf


def children(r, regions, ids, parent, base, decoys):
    """a child list of the windows `ids` (address, id) shuffled among decoy nodes (ids from decoys) from base on; returns its head"""
    items = list(ids) + [(base + NODE * i, r.choice(decoys + [r.getrandbits(32)])) for i in range(r.choice([0, 1, 2, 3]))]
    r.shuffle(items)
    for i, (addr, ident) in enumerate(items):
        if addr not in regions:
            regions[addr] = node(r, ident, 0)
            put(regions[addr], 0x60, 'I', parent)
        put(regions[addr], 0x54, 'I', items[i + 1][0] if i + 1 < len(items) else 0)
        put(regions[addr], 0x8c, 'I', ident)
    return items[0][0] if items else 0


def case(r):
    r.deal(OPS)
    log = log_region()
    regions = {LOG: log}
    wins = shown_env(r, regions, log)
    count = r.choice([0, 1, 2, 3, 4, 6])
    park_regions, handles, things = park(r, count=count, kinds=(1, 1, 1, 2, 3), defs=False, cells=True, pool=POOL)
    regions.update(park_regions)
    pool = bytearray(regions[POOL])
    for i in range(count):
        put(pool, THING * i + 0x30, 'i', r.choice([0, 1, 2, 3, 4, 5, 10, 21, 22, r.randrange(23)]))
    regions[POOL] = pool
    regions[RAM + 0x1da720] = struct.pack('<H', r.choice(handles + [0]) if handles else 0)
    kids = [(KIDS + NODE * k, 0x4a68d2 + k) for k in range(3)]
    for addr, _ in kids:
        window(r, regions, addr, PANEL, r.choice([0, 0, wins[0]]))
    regions[KVTABLE] = vtable({7: 1})
    window(r, regions, ROOT, 0, 0)
    window(r, regions, PANEL, ROOT, 0)
    put(regions[PANEL], 0x5c, 'I', children(r, regions, kids, PANEL, DECOYS + 0x1000, [0, 0x4a68d1, 0x4a68d5]))
    put(regions[ROOT], 0x5c, 'I', children(r, regions, [(PANEL, 0x4a68d1)] if r.random() < 0.9 else [], ROOT, DECOYS, [0, 0x4a68d0, 0x4a68d2, 0x4a68d4]))
    regions[0x7cb398] = struct.pack('<I', ROOT)
    return Call(CALLS['ShowTopThree'], [], regions, ret='v')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
