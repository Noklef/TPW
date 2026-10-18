# The engine's objects created at start-up against the originals: the block at 0x879d54 (src/game/core/globals: 0x56b6c0), the procedural
# terrain texture object at 0x879d60 (src/game/terrain/procedural: 0x56ddf0), the blocks at 0x8bcbb8 and 0x8c47d4 (core/globals: 0x5804b0,
# 0x58aff0), the sprite queue at 0x8c47a0 (render/spritequeue: 0x58a430), the renderer at 0x87a6d0 with its pool (render/renderer: 0x5760a0;
# the pool from the tracked GlobalAlloc table, its count 0-6 or full at 0x8000) and the object at 0x8bba48 added to the live objects (core/vwrap:
# 0x57c360; their count 0-0x200), each with no object yet mostly (else one), on fncmp's heap whose allocation fails now and then (the first, or the
# renderer's pool: the error then pushed onto the engine's list at 0x879ad0, with a count of 0 to 0x40 and the engine debug flag at 0x8bd508 half
# the time: the log 0x564650 is fncmp's one-argument cdecl logger, its text hashed, as in engineerrors.py).
# usage: enginecreate.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Block,Texture,...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, Bench, Call, _cdecl_log, global_heap, heap, log_region, put, strings  # noqa: E402

B = Bench([(0x56b6c0, 0x56b755), (0x56ddf0, 0x56de86), (0x56af20, 0x56af9c), (0x56a770, 0x56ae60), (0x67a400, 0x690000),
           (0x5804b0, 0x5804f7), (0x58aff0, 0x58b034), (0x58a430, 0x58a48e), (0x5760a0, 0x5761a6), (0x579b40, 0x579c13),
           (0x57c360, 0x57c49b), (0x5775d0, 0x577600)],
          hooks={0x564650: _cdecl_log})
# name: (address, slot, the first allocation's size)
CALLS = {'Block': (0x56b6c0, 0x879d54, 0x2d8), 'Texture': (0x56ddf0, 0x879d60, 0x12974), 'Block8bcbb8': (0x5804b0, 0x8bcbb8, 0x14004),
         'Block8c47d4': (0x58aff0, 0x8c47d4, 0xc), 'SpriteQueue': (0x58a430, 0x8c47a0, 0x2410), 'Renderer': (0x5760a0, 0x87a6d0, 0x80 + 0x24010),
         'Live8bba48': (0x57c360, 0x8bba48, 0xa8)}
OPS = os.environ.get('OPS', ','.join(CALLS)).split(',')
COUNT, PAIRS = 0x8bb214, 0x87b1f8
LIVE, LIVE_COUNT = 0x87a6d8, 0x87aed8


def case(r):
    name = r.deal(OPS)
    address, slot, size = CALLS[name]
    log = log_region()
    strings(log, 0x3f, 1)
    errors = bytearray(r.bytes(0x204))
    struct.pack_into('<i', errors, 0x200, r.choice([r.randrange(0x41), 0, 0x3f, 0x40]))
    flags = r.getrandbits(32) & ~0x1000000 | (0x1000000 if r.random() < 0.5 else 0)
    fail = r.choice([1, 2, 2]) if name == 'Renderer' and r.random() < 0.45 else 1 if r.random() < 0.3 else 0
    regions = {slot: struct.pack('<I', RAM if r.random() < 0.2 else 0), RAM: r.bytes(0x10), HEAP: heap(size + 0x40, fail),
               0x879ad0: errors, 0x8bd508: struct.pack('<I', flags), LOG: log}
    if name == 'Renderer':
        regions.update(global_heap())
        regions[COUNT] = struct.pack('<I', r.choice([0, 1, 2, 6, 0x8000]))
        regions[PAIRS] = r.bytes(0x40)
    if name == 'Live8bba48':
        count = r.choice([0, 1, 5, 0x1ff, 0x200, r.randrange(0x200)])
        regions[LIVE] = r.bytes(0x800)
        regions[LIVE_COUNT] = struct.pack('<I', count)
    return Call(address, [], regions, conv='c', ret='a')


if __name__ == '__main__':
    B.main(case)
