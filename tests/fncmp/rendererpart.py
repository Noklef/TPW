# The renderer part's constructor and destructors (src/game/render/renderer: 0x48f150, 0x48f300, 0x48f2e0) against the originals: a part
# of random bytes; the live objects list from empty to full (the set's virtual base listed or not), the tracked blocks' table (as
# spritequeue.py), the heap (now and then failing the set's array: a fault in both); for the destructors the set's table the exe's
# (0x6ff318) or made up (other virtual base offsets), its array's block tracked or not, the image's textures none or an empty array and its
# pixels none or a tracked block, any flags.
# Layout (0x48f420) and the text parts' TextLayout (0x48fc40, src/game/render/textparts): see layout() below.
# usage: rendererpart.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Ctor,Dtor,ScalarDtor,Layout,TextLayout)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, Bench, Call, global_heap, heap, log_region, put, vtable  # noqa: E402

CALLS = {'Ctor': 0x48f150, 'Dtor': 0x48f300, 'ScalarDtor': 0x48f2e0, 'Layout': 0x48f420, 'TextLayout': 0x48fc40}
B = Bench([(0x48f150, 0x48f290), (0x48f2e0, 0x48f3d0), (0x48f420, 0x48f5b7), (0x48fc40, 0x48fdac), (0x67a830, 0x67a860), (0x5775d0, 0x577660), (0x579b40, 0x579c30), (0x588320, 0x588420), (0x588d70, 0x588e50),
           (0x58a4f0, 0x58a560), (0x58dc50, 0x58dd70), (0x590b90, 0x590c50), (0x590a70, 0x590a75), (0x588ca0, 0x588d6e), (0x6597b0, 0x6598ed),
           (0x659994, 0x6599bc), (0x659d00, 0x659d53), (0x659d90, 0x659dbc), (0x65a9d0, 0x65aa00), (0x66bff7, 0x66c01b), (0x67afa0, 0x67b0d0)])
PART, TABLE, EMPTY = RAM + 0x100, RAM, RAM + 0x300
LIVE, LIVE_COUNT, COUNT, PAIRS = 0x87a6d8, 0x87aed8, 0x8bb214, 0x87b1f8
BLOCKS = [HEAP + 0x40, HEAP + 0x80]


def layout(r, text=False):
    """Layout (0x48f420): a part of random bytes with a recording vtable (slot 10 the pixels' allocation), +0x44 clear or set; flags of
    0x10 / 2 / 4 / other bits; the info none or random bytes with its rectangle (random words) at +0x14 and the s16 +6 about 0; the GUI's
    size words any, 0 or negative. TextLayout (text): slot 9 recorded too, the flags or'd with some of 0x7fd00 (0x40000 the colour, 0x100
    slot 9), the info's +0x10 a record of random bytes (its dword +0 0 or not, the colour bytes +0x10 0, 0xff or any) or 0, the x87's
    precision 24, 53 or 64 bits."""
    vt, info, rect = RAM + 0x400, RAM + 0x500, RAM + 0x600
    part = put(bytearray(r.bytes(0x100)), 0, 'I', vt)
    put(part, 0x44, 'H', r.choice([0, 0, 1, 0xffff]))
    words = lambda: r.choice([0, 1, -1, 768, 1536, 2048, -32768, 32767, r.randrange(-32768, 32768), r.randrange(-3000, 3000)])
    gui = put(put(bytearray(8), 0, 'h', r.choice([1536, 1536, 768, 600, 0, -5, words()])), 4, 'h', r.choice([2048, 2048, 1024, 800, 0, -7, words()]))
    regions = {LOG: log_region(), vt: vtable({10: 1, 9: 1} if text else {10: 1}, 12), RAM + 0x100: part, 0xfaa5c0: gui,
               rect: b''.join(struct.pack('<h', words()) for _ in range(8)), info: put(bytearray(r.bytes(0x20)), 0x14, 'I', rect)}
    put(regions[info], 6, 'h', r.choice([-1, 0, 1, 2, 40, 1000, words()]))
    flags = r.choice([0, 2, 4, 6, 0x10, 0x12, 0x14, 0x16, 0x16, 1, 8, 0x20, 0x13, r.getrandbits(32), r.getrandbits(5)])
    if text:
        record, bits = RAM + 0x700, [0x100, 0x400, 0x800, 0x1000, 0x2000, 0x4000, 0x8000, 0x10000, 0x20000, 0x40000]
        flags = r.choice([0, 0x10, 0x57, 0x10057, r.getrandbits(5)]) | sum(b for b in bits if r.random() < 0.4)
        if r.random() < 0.1:
            flags = r.choice([0, 0x200, 0x80000, r.getrandbits(32)])
        rec = bytearray(r.bytes(0x14))
        put(rec, 0, 'I', r.choice([0, r.getrandbits(32)]))
        rec[0x10:0x14] = bytes(r.choice([0, 0xff, 1, 0x80, r.getrandbits(8)]) for _ in range(4))
        regions[record] = rec
        put(regions[info], 0x10, 'I', r.choice([record] * 9 + [0]))
        return Call(0x48fc40, [RAM + 0x100, flags, r.choice([info] * 9 + [0])], regions, conv='t', ret='v', cw=r.choice([0x7f, 0x7f, 0x27f, 0x37f]))
    return Call(0x48f420, [RAM + 0x100, flags, r.choice([info, info, info, 0])], regions, conv='t', ret='v')


def case(r):
    op = r.deal(OPS)
    if op in ('Layout', 'TextLayout'):
        return layout(r, op == 'TextLayout')
    regions = {HEAP: heap(fail=1 if r.random() < 0.03 else 0), **global_heap(), EMPTY: bytes(8)}
    part = bytearray(r.bytes(0x100))
    off = 0x20
    if op != 'Ctor':
        table = 0x6ff318
        if r.random() < 0.4:
            off = r.choice([0x24, 0x28, 0x30])
            table = TABLE
            regions[TABLE] = struct.pack('<2I', r.getrandbits(32), off)
        put(part, 0xb4, 'I', table)
        put(part, 0xb4 + 0x14, 'I', r.choice(BLOCKS + [0, r.getrandbits(32) | 0x80000000]))
        put(part, 0x9c, 'I', r.choice([0, 0, EMPTY + 4]))
        put(part, 0x9c + 4, 'I', r.choice([0, 0] + BLOCKS))
        put(part, 0x9c + 0x16, 'H', 0)
    regions[PART] = part
    count = r.choice([0, 1, 2, 3, 0x10])
    regions[COUNT] = struct.pack('<I', count)
    pairs = bytearray(r.bytes(0x80))
    for k in range(min(count, 2)):
        if r.random() < 0.6:
            put(pairs, 8 * k, '2I', BLOCKS[k], BLOCKS[k])
    regions[PAIRS] = pairs
    live_count = r.choice([0, 1, 2, 5, 0x1ff, 0x200, r.randrange(0x200)])
    live = bytearray(r.bytes(0x800))
    if live_count and op != 'Ctor' and r.random() < 0.6:
        put(live, 4 * r.randrange(min(live_count, 0x200)), 'I', PART + 0xb4 + off)
    regions[LIVE] = live
    regions[LIVE_COUNT] = struct.pack('<I', live_count)
    if op == 'Ctor':
        return Call(CALLS[op], [PART, r.getrandbits(32)], regions, conv='t', ret='a')
    if op == 'Dtor':
        return Call(CALLS[op], [PART], regions, conv='t')
    return Call(CALLS[op], [PART, r.choice([0, 1, 1, 2, 3, 0xffffffff])], regions, conv='t', ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
