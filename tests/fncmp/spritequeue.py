# The sprite queue's appends and its release (src/game/render/spritequeue: 0x58a550, 0x58a6b0, 0x58a6e0, 0x58a490) against the
# originals: lists and frame sets from empty to full, stamps matching or not, the second one's counts overrun included (0x101, 0x102); no
# queue, or one whose texture array (as texturemem.py) is set or not. A sprite set's deleting destructor (0x491f20, and through its thunk
# 0x491f10): virtual base offsets inside the set, its tracked array's block tracked or not, the virtual base among the live objects or not,
# flags 0, 1 and others. Its destructor (0x48f280) likewise.
# usage: spritequeue.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Ready,Add,Add2,Free,SetDelete,SetDeleteThunk,SetDtor)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import texturemem  # noqa: E402
from fncmp import Bench, Call, HEAP, LOG, RAM, global_heap, heap, log_region, put  # noqa: E402

B = Bench([(0x58a550, 0x58a5b9), (0x58a6b0, 0x58a6de), (0x58a6e0, 0x58a712), (0x58a490, 0x58a4ec), (0x590a70, 0x590a75), (0x590b90, 0x590c39),
           (0x579b40, 0x579c13), (0x58dc50, 0x58dd5e), (0x67afa0, 0x67b060), (0x67acb0, 0x67acb1),
           (0x491f10, 0x491f7d), (0x58a520, 0x58a548), (0x577600, 0x57764b), (0x48f280, 0x48f2d8)])
CALLS = {'Ready': 0x58a550, 'Add': 0x58a6b0, 'Add2': 0x58a6e0, 'Free': 0x58a490, 'SetDelete': 0x491f20, 'SetDeleteThunk': 0x491f10, 'SetDtor': 0x48f280}
QUEUE, ARRAY = RAM + 0x8000, RAM + 0x6000   # (texturemem.py's regions below RAM + 0x4000)


def free(r):
    regions = {HEAP: heap(), **global_heap(), LOG: log_region()}
    if r.random() < 0.1:
        regions[0x8c47a0] = bytes(4)
        return Call(0x58a490, [], regions, conv='c', ret='a')
    queue = bytearray(r.bytes(0x2410))
    if r.random() < 0.8:
        n = r.choice([0, 1, 2, 3])
        regions[ARRAY] = struct.pack('<I', n) + texturemem.textures(r, regions, ARRAY + 4, n + 1)
        put(queue, 0x2400, 'I', ARRAY + 4)
    else:
        put(queue, 0x2400, 'I', 0)
    regions[QUEUE] = queue
    regions[0x8c47a0] = struct.pack('<I', QUEUE)
    return Call(0x58a490, [], regions, conv='c', ret='a')


SET, LIVE, LIVE_COUNT, COUNT, PAIRS = RAM + 0x100, 0x87a6d8, 0x87aed8, 0x8bb214, 0x87b1f8


def set_delete(r, op):
    off = r.choice([0x24, 0x28, 0x30, 0x3c, 0x40])   # (the virtual base's offset in the set, from its table at RAM)
    regions = {HEAP: heap(), **global_heap(), RAM: struct.pack('<2I', r.getrandbits(32), off)}
    obj = bytearray(r.bytes(0x60))
    put(obj, 0, 'I', RAM)
    block = r.choice([HEAP + 0x40, HEAP + 0x80, 0, r.getrandbits(32)])
    put(obj, 0x14, 'I', block)
    regions[SET] = obj
    count = r.choice([0, 1, 2, 3, 0x10])
    regions[COUNT] = struct.pack('<I', count)
    pairs = bytearray(r.bytes(0x80))
    if count and r.random() < 0.6:
        put(pairs, 8 * r.randrange(count), 'I', block)
    regions[PAIRS] = pairs
    live_count = r.choice([0, 1, 2, 5, 0x1ff, 0x200, r.randrange(0x200)])
    live = bytearray(r.bytes(0x800))
    if live_count and r.random() < 0.6:
        put(live, 4 * r.randrange(min(live_count, 0x200)), 'I', SET + off)
    regions[LIVE] = live
    regions[LIVE_COUNT] = struct.pack('<I', live_count)
    flags = r.choice([0, 1, 1, 2, 3, 0xffffffff])
    if op == 'SetDtor':
        return Call(0x48f280, [SET], regions, conv='t')
    if op == 'SetDeleteThunk':   # (this lowered by the dword before it, to the set's +0x20)
        regions[RAM + 0x1fc] = struct.pack('<I', (RAM + 0x200 - SET - 0x20) & 0xffffffff)
        return Call(0x491f10, [RAM + 0x200, flags], regions, conv='t', ret='a')
    return Call(0x491f20, [SET + 0x20, flags], regions, conv='t', ret='a')


def case(r):
    op = r.deal(OPS)
    if op == 'Free':
        return free(r)
    if op.startswith('Set'):
        return set_delete(r, op)
    queue = bytearray(r.bytes(0x4010))
    if op == 'Ready':
        stamp = r.bytes(8)
        obj = bytearray(r.bytes(0x40))
        put(obj, 0xc, '8s', stamp if r.random() < 0.3 else r.bytes(8))
        if r.random() < 0.2:
            put(obj, 0xc, 'I', struct.unpack_from('<I', stamp)[0])
        put(queue, 0x240c, 'I', r.choice([0, 1, 0x7e, 0x7f, 0x80, 0x81, r.randrange(0x80)]))
        regions = {RAM: obj, RAM + 0x1000: queue, 0x8c47a0: struct.pack('<I', RAM + 0x1000), 0x8bcc40: stamp}
        return Call(0x58a550, [RAM + 0x20], regions, conv='t', ret='a')
    counts = [0, 1, 0x7f, 0x80, 0x100, 0x101, 0x102, 0x103, 0x7ff, 0x800, 0x801, r.randrange(0x800)]
    put(queue, 0x2404, 'I', r.choice(counts))
    put(queue, 0x2408, 'I', r.choice(counts))
    return Call(CALLS[op], [RAM, r.getrandbits(32)], {RAM: queue}, conv='t', ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
