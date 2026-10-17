# The texture sets' constructor and getters (src/game/gfx/texset: 0x575bd0, 0x575f90, 0x575fa0)
# against the originals: any set bytes, textures 0 to 7 (and negative for the slot's address). The release (0x575f00): users 1, 2, 0 or
# negative; flag 0x8000 with one shared texture array (as texturemem.py, its count 0-3; or none), else 0-4 textures (the count's low word,
# 0 or negative) each released from the manager at 0x87a268 (0-4 slots of their arrays, users 1, 2 or 0, or another array); the list a
# tracked global block (the pairs at 0x87b1f8) or not.
# usage: texset.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Ctor,Texture,TextureSlot,Release)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import texturemem  # noqa: E402
from fncmp import Bench, Call, HEAP, LOG, RAM, global_heap, heap, log_region, put  # noqa: E402

B = Bench([(0x575bd0, 0x575be5), (0x575f90, 0x575f9d), (0x575fa0, 0x575fad), (0x575f00, 0x575f8d), (0x575ad0, 0x575b7a), (0x579b80, 0x579c13),
           (0x590a70, 0x590a75), (0x590b90, 0x590c39), (0x58dc50, 0x58dd5e), (0x67afa0, 0x67b0d0), (0x67acb0, 0x67acb1)])
CALLS = {'Ctor': 0x575bd0, 'Texture': 0x575f90, 'TextureSlot': 0x575fa0, 'Release': 0x575f00}
TEXTURES = RAM + 0x40
SET, LIST, MANAGER, ARRAYS = RAM + 0x10000, RAM + 0x10100, RAM + 0x20000, RAM + 0x5000   # (texturemem.py's regions below RAM + 0x4000)
COUNT, TABLE = 0x8bb214, 0x87b1f8


def release(r):
    regions = {HEAP: heap(), **global_heap(), LOG: log_region()}
    s = bytearray(r.bytes(0x10))
    put(s, 8, 'I', LIST)
    put(s, 0xe, 'H', r.choice([1, 1, 1, 2, 0, 0xffff]))
    lst = bytearray(r.bytes(0x20))
    if r.random() < 0.3:
        put(s, 0xc, 'H', 0x8000 | r.choice([0, 1, 3]))
        if r.random() < 0.85:
            m = r.choice([0, 1, 2, 3])
            regions[ARRAYS] = struct.pack('<I', m) + texturemem.textures(r, regions, ARRAYS + 4, max(m, 1))
            put(lst, 0, 'I', ARRAYS + 4)
        else:
            put(lst, 0, 'I', 0)
    else:
        n = r.choice([0, 1, 2, 3, 4])
        put(s, 0xc, 'H', r.choice([n, n, n, 0, 0x7fff & r.getrandbits(3)]) if r.random() < 0.95 else 0)
        slots = bytearray(r.bytes(0x10 * n + 0x10))
        for k in range(n):
            if r.random() < 0.85:
                m = r.choice([1, 2, 3])
                base = ARRAYS + 0x400 * k
                regions[base] = struct.pack('<I', m) + texturemem.textures(r, regions, base + 4, m)
                put(slots, 0x10 * k + 8, 'I', base + 4)
                put(lst, 4 * k, 'I', base + 4)
            else:
                put(slots, 0x10 * k + 8, 'I', 0)
                put(lst, 4 * k, 'I', ARRAYS + 0x2004)
            put(slots, 0x10 * k + 0xc, 'I', r.choice([1, 1, 2, 0]))
        regions[MANAGER] = slots
        regions[MANAGER - 0x10] = r.bytes(12) + struct.pack('<I', 2)
        regions[MANAGER + 0x10000] = struct.pack('<III', n, r.getrandbits(32), r.getrandbits(32))
        regions[0x87a268] = struct.pack('<I', MANAGER)
    regions[SET] = s
    regions[LIST] = lst
    pairs = r.choice([0, 1, 2, 3])
    table = bytearray(r.bytes(8 * 4))
    if pairs and r.random() < 0.85:
        put(table, 8 * r.randrange(pairs) + 4, 'I', LIST)
    regions[TABLE] = table
    regions[COUNT] = struct.pack('<I', pairs)
    return Call(0x575f00, [SET], regions, conv='t', ret='a')


def case(r):
    op = r.deal(OPS)
    if op == 'Release':
        return release(r)
    mem = bytearray(r.bytes(0x60))
    put(mem, 8, 'I', TEXTURES)
    if op == 'Ctor':
        return Call(0x575bd0, [RAM], {RAM: mem}, conv='t', ret='a')
    i = r.randrange(8) if op == 'Texture' else r.choice([r.randrange(8), -1, r.getrandbits(32)])
    return Call(CALLS[op], [RAM, i], {RAM: mem}, conv='t', ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
