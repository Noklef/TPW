# The texture cache's touch of an item and its trace (src/game/gfx/texcache: 0x58c9f0, 0x58e010) against the originals: an item chain of 1-3 (flag 0x800000), flags with the
# 0x400 / 0x1000 / 0x2000000 / 0x3000 / 0x4000 ones often, the frame equal or not, the virtual base's slot byte at 0xff or below, weights around the item's, NaNs. The trace's
# C library calls are recorders (no file is written), fopen answers a handle or null; the record's bytes are hashed, not its address.
# usage: texcachetouch.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Touch,Trace)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, blob, fb, log_region, put, set_return, strings  # noqa: E402

FOPEN, FWRITE, FFLUSH, FCLOSE = 11, 12, 13, 14   # driver.cpp's kTexCacheTraceEnv
HOOKS = {0x67b8a0: _recorder(FOPEN, 2, cdecl=True), 0x67b6d0: _recorder(FWRITE, 4, cdecl=True), 0x67d070: _recorder(FFLUSH, 1, cdecl=True), 0x67b620: _recorder(FCLOSE, 1, cdecl=True)}
B = Bench([(0x58c9f0, 0x58cb33), (0x58e010, 0x58e105), (0x58dc50, 0x58dd5e), (0x58cb40, 0x58cb92), (0x58b380, 0x58b390)], hooks=HOOKS)
OPS = os.environ.get('OPS', 'Touch,Touch,Touch,Trace').split(',')
CACHE, ITEMS, TABLE, BASE, FRAME, OWNER, ENTRY = RAM, RAM + 0x2000, RAM + 0x2200, RAM + 0x2300, RAM + 0x2400, RAM + 0x2440, RAM + 0x2480
FILE = 0x12340000
SLOT, FRAMEAT, FLAGS08, TRACE, MORE = 0x879d48, 0x87a248, 0x8bd508, 0x8bd550, 0x8bcbc8


def weight(r, near):
    return r.choice([fb(r.uniform(0, 2)), fb(r.uniform(0, 2)), near, fb(0.5), r.f(), 0x7fa00000, 0x7fc00000])


def globals_(r, trace):
    return {SLOT: struct.pack('<I', r.randrange(0, 0x10)), FRAMEAT: struct.pack('<I', FRAME), FLAGS08: r.choice([0, 0x40000000, 0x80000000, r.getrandbits(32)]).to_bytes(4, 'little'),
            MORE: r.bytes(4), TRACE: bytes([(r.getrandbits(8) | 0x80) if trace else r.getrandbits(8) & 0x7f]) + r.bytes(0x1f)}


def item_at(r, count, k, frame):
    flags = r.getrandbits(32)
    for bit, p in ((0x2000000, 0.1), (0x400, 0.5), (0x1000, 0.5), (0x2000, 0.4), (0x4000, 0.5), (0x80000, 0.5)):
        flags = flags | bit if r.random() < p else flags & ~bit
    flags = flags | 0x800000 if k + 1 < count else flags & ~0x800000
    item = bytearray(r.bytes(0x60))
    put(item, 0, 'I', TABLE)
    put(item, 8, 'I', flags)
    put(item, 0xc, 'I', frame if r.random() < 0.4 else r.getrandbits(32))
    put(item, 0x10, 'HH', r.choice([0, 1, 7, 8, 0x7fff, 0xffff, r.getrandbits(16)]), r.choice([0, 1, 7, 8, 0x7fff, 0xffff, r.getrandbits(16)]))
    put(item, 0x18, 'I', OWNER + 0x10 * k)
    put(item, 0x30, 'I', r.choice([fb(r.uniform(0, 2)), fb(1.0), r.f(), 0x7fa00000]))
    put(item, 0x3c, 'I', r.choice([0, r.getrandbits(32) | 1]))
    return item


def case(r):
    op = r.deal(OPS)
    log = log_region()
    strings(log, FOPEN, 3)
    blob(log, FWRITE, 0, 0xf)
    set_return(log, FOPEN, 0 if r.random() < 0.1 else FILE)
    cache = bytearray(r.bytes(0x1900))
    put(cache, 0x18cc, 'I', r.getrandbits(32) | 0x20000000 if r.random() < 0.8 else r.getrandbits(32))
    put(cache, 0x18e4, 'I', r.choice([0, 1, 3, 7, 0xf, 0x1f, r.getrandbits(32)]))
    put(cache, 0x18c4, 'I', r.getrandbits(32))
    frame = r.getrandbits(32)
    regions = {CACHE: cache, TABLE: struct.pack('<2I', 0, BASE - ITEMS) + r.bytes(8), FRAME: struct.pack('<2I', 0, frame), 
               ENTRY: r.bytes(0x10), LOG: log}
    regions.update(globals_(r, op == 'Trace' or r.random() < 0.3))
    # Each item's owner: the entry (its flag set through it), then a list of the item alone. The items share a table, so their virtual bases are 0x60 apart.
    regions[OWNER] = b''.join(struct.pack('<2I', ENTRY, ITEMS + 0x60 * k) + r.bytes(8) for k in range(3))
    if op == 'Trace':
        regions[ITEMS] = item_at(r, 1, 0, frame)
        return Call(0x58e010, [CACHE, ITEMS], regions, conv='t', ret='v')
    count = r.choice([1, 1, 2, 3])
    items = bytearray()
    for k in range(count):
        items += item_at(r, count, k, frame)
    regions[ITEMS] = items
    base = bytearray(r.bytes(0x100))
    if r.random() < 0.3:
        for k in range(3):
            base[0x60 * k:0x60 * k + 0x10] = b'\xff' * 0x10
    regions[BASE] = base
    w = weight(r, struct.unpack_from('<I', items, 0x30)[0])
    item = 0 if r.random() < 0.03 else ITEMS
    return Call(0x58c9f0, [CACHE, item, w], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
