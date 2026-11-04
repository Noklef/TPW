# The texture coordinate probes (src/game/render/uvprobe: 0x583e90, 0x584310) against the originals, the texture's calls, the cache's and the device's recorders; TexFormat_Classify a stand-in (tools/fncmp/driver.cpp's uvClassifyStandIn, the same) keeping each try's 60 lines
# read back and answering from a table: answers that are all above, all below, or mixed, with or without 0s. The texture 16 or 32 bits, the driver kind 4 or not, its flag 0x1000 set by the lock or not, its allocation failing now and then.
# usage: uvprobe.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, LOG_RETURNS, RAM, Bench, Call, _recorder, fill, fill_this, heap, log_fold, log_region, put, set_return, strings, vtable  # noqa: E402

RELEASE_ITEMS, DEVICE0, ALLOC, LOCK, TOUCH, UPDATE, CLASSIFY, UNLOCK = 20, 21, 22, 23, 24, 25, 26, 27
TABLE, OUT, DEVICE, VT, SETTINGS, PIXELS = RAM, RAM + 0x100, RAM + 0x1900, RAM + 0x1a00, RAM + 0x2000, RAM + 0x10000
COUNT = TABLE + 0x80
_classify_record = _recorder(CLASSIFY, 1)


def _classify(e):
    n = e.u32(COUNT)
    e.write(COUNT, struct.pack('<I', n + 1))
    e.write(OUT + 0xf0 * n, e.read(e.u32(e.r['ESP'] + (4 if e.tail else 0)), 0xf0))
    e.write(LOG + LOG_RETURNS + 8 * CLASSIFY, struct.pack('<II', 1, e.u32(TABLE + 4 * n)))
    _classify_record(e)


B = Bench([(0x583e90, 0x584310), (0x584310, 0x584790), (0x590a00, 0x590a70)],
          hooks={0x58dd60: _recorder(RELEASE_ITEMS, 1), 0x583720: _recorder(DEVICE0, 0, cdecl=True), 0x590a80: _recorder(ALLOC, 2), 0x590d40: _recorder(LOCK, 0), 0x58c9f0: _recorder(TOUCH, 2), 0x58cf40: _recorder(UPDATE, 0), 0x583ba0: _classify, 0x590a70: _recorder(UNLOCK, 0)})
OPS = ['Down', 'Across']
ROUTES = ['above', 'below', 'mixed', 'zeros', 'zeros', 'zeros', 'once', 'mute']


def answers(r, route):
    if route == 'above':
        return [r.randrange(1, 100) for _ in range(24)]
    if route == 'below':
        return [-r.randrange(1, 100) for _ in range(24)]
    if route == 'mixed':
        return [r.choice([-1, 1, 5, -7]) for _ in range(24)]
    if route == 'once':   # (a 0 in the first search only: the second keeps nothing)
        first = [r.choice([-1, 1]) for _ in range(23)]
        first.insert(r.randrange(12), 0)
        return first
    if route == 'mute':   # (0s in the second search but none in the first)
        return [r.choice([-1, 1]) for _ in range(12)] + [r.choice([0, 1, -1]) for _ in range(12)]
    return [r.choice([0, 0, 1, -1]) for _ in range(24)]


def case(r):
    op = r.deal(OPS)
    log = log_region()
    log_fold(log)
    strings(log, 16, 0, 4)
    strings(log, CLASSIFY, 0, 1)
    fill(log, 16, 2, r.bytes(0x70), which=2, records=0x1c)
    set_return(log, LOCK, PIXELS)
    fill_this(log, LOCK, struct.pack('<I', r.choice([0x51b0000, 0x51b1000, r.getrandbits(32)])), 8)
    fill_this(log, LOCK, r.bytes(4), 0x1c, which=1)
    index = r.randrange(8)
    settings = put(bytearray(r.bytes(0x40f0)), 4, 'I', index)
    table = bytearray(r.bytes(0x20))
    put(table, 4 * index, 'I', r.choice([2, 2, 1, 3]))
    regions = {LOG: log, TABLE: struct.pack('<24i', *answers(r, r.deal(ROUTES))) + bytes(0x20), OUT: bytes(0xf0 * 24), DEVICE: struct.pack('<I', VT), VT: vtable({2: 1, 3: 2, 6: 3, 0x10: 3, 0x11: 7}, 0x20), SETTINGS: settings, PIXELS: bytes(0x10000),
               HEAP: heap(0x100, 1 if r.random() < 0.05 else 0), 0x701e18: table, 0x879ab0: struct.pack('<I', DEVICE), 0x8bd47c: r.bytes(4), 0x8bd4c4: r.bytes(0x20), 0x8bd524: struct.pack('<I', r.choice([1, 4])), 0x8c1644: r.bytes(4), 0x8c4b44: r.bytes(4)}
    return Call(0x583e90 if op == 'Down' else 0x584310, [SETTINGS], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
