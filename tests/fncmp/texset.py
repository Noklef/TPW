# The texture sets' constructor and getters (src/game/gfx/texset: 0x575bd0, 0x575f90, 0x575fa0)
# against the originals: any set bytes, textures 0 to 7 (and negative for the slot's address). The release (0x575f00): users 1, 2, 0 or
# negative; flag 0x8000 with one shared texture array (as texturemem.py, its count 0-3; or none), else 0-4 textures (the count's low word,
# 0 or negative) each released from the manager at 0x87a268 (0-4 slots of their arrays, users 1, 2 or 0, or another array); the list a
# tracked global block (the pairs at 0x87b1f8) or not.
# The set-up (0x575c60, and 0x575ee0 through a pair): listed already (users, flag 0x30000 on 0-2 textures) or not; one blank texture
# (definition 0x80; Texture_Alloc of sizes up to 0x40) or the textures loaded now or put on the waiting list (0x8bd550's 0x40000000;
# that list's count 0, 1, at 0x100 or past it); the heap failing the list's, the array's or the pixels' allocation now and then. The
# loads (0x575e10, and 0x575b90 over 0-2 waiting sets): 0-3 names (the file's, others, the manager's) under texfile.py's file split into
# [0x786de4], the folder and the name, the manager's slots as texman.py's, the definition's load flags; the memory source holds four
# files (texfile.py's exact) so that each load reads one.
# usage: texset.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Ctor,Texture,TextureSlot,Release,Init,Pair,LoadTextures,LoadDeferred)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import texfile  # noqa: E402
import texman  # noqa: E402
import texturemem  # noqa: E402
import vfs  # noqa: E402
import wctimage  # noqa: E402
from fncmp import Bench, Call, HEAP, LOG, RAM, global_heap, heap, log_region, put  # noqa: E402

B = Bench(wctimage.merged(texman.RANGES + [(0x575bd0, 0x575be5), (0x575f90, 0x575f9d), (0x575fa0, 0x575fad), (0x575f00, 0x575f8d), (0x575ad0, 0x575b7a),
                                            (0x579b80, 0x579c13), (0x590a70, 0x590a75), (0x590b90, 0x590c39), (0x58dc50, 0x58dd5e), (0x67afa0, 0x67b0d0),
                                            (0x67acb0, 0x67acb1), (0x575b90, 0x575bc3), (0x575c60, 0x575ef3), (0x590a80, 0x590b88), (0x56af20, 0x56af9c)]))
CALLS = {'Ctor': 0x575bd0, 'Texture': 0x575f90, 'TextureSlot': 0x575fa0, 'Release': 0x575f00, 'Init': 0x575c60, 'Pair': 0x575ee0,
         'LoadTextures': 0x575e10, 'LoadDeferred': 0x575b90}
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


SET2, INFO, NAMES, DEF, PAIR, LIST2, TEXS = (RAM + 0x94000, RAM + 0x94100, RAM + 0x94200, RAM + 0x94400, RAM + 0x94480, RAM + 0x94500,
                                            RAM + 0x94600)
FOLDER, OTHER = texman.STR, texman.STR + 0x200
DEFERRED = 0x87a270   # (the sets waiting, then their count at 0x87a670)


def def_bytes(r):
    return bytes([r.getrandbits(8) | (0x80 if r.random() < 0.3 else 0) if r.random() < 0.5 else r.getrandbits(8) & 0x7f, r.getrandbits(8)]) + r.bytes(2)


def files(r, first, fail):
    """texfile.py's scene (the heap failing at fail), its file's path split into [0x786de4], the info's folder (+4: none, or one ending
    at a backslash) and a name of at most 0x13 bytes; 0-3 names (0x14 bytes each: the file's, another, or the manager's), the manager
    at [0x87a268] with 0-3 slots (the folder, null or another; the names) and its load flags 0, 2, 4 or 8. Returns the regions, the info
    (missing its count) and the count of names."""
    regions = texfile.scene(r, 0, first)
    regions[HEAP] = heap(0x40000, fail=fail)   # (room for three loads' blocks: they are never reused)
    vfs.memory_source(r, regions[LOG], regions, b''.join(texfile.exact(r) for _ in range(4)))   # (a load after a load reads on)
    full = regions[vfs.NAME].split(bytes(1))[0]
    lo = max(len(full) - 0x13, 0)
    s = r.randrange(lo, len(full) + 1)
    folder, base, name = None, full[:s], full[s:]
    cuts = [c for c in range(len(full)) if full[c:c + 1] == b'\\' and c + 1 >= lo]
    if cuts and r.random() < 0.5:
        c = r.choice(cuts)
        i = r.randrange(c + 1)
        base, folder, name = full[:i], full[i:c], full[c + 1:]
    regions[0x786de4] = base + bytes(1)
    others = [b'a.tga', name[:-1], name + b'z', b'tex.wct']   # (each with a '.' to find: a path without one faults in both)
    n = r.choice([0, 1, 2, 3])
    names = bytearray(b''.join(b'q.tga' + bytes(1) + r.bytes(0x0e) for _ in range(4)))   # (past the count too: a count of 1 over none)
    words = []
    for k in range(n):
        w = r.choice([name, name, r.choice(others)])
        names[0x14 * k:0x14 * k + len(w) + 1] = w + bytes(1)
        words.append(w)
    regions[NAMES] = bytes(names)
    strings = bytearray(0x800)
    strings[0:len(folder or b'') + 1] = (folder or b'') + bytes(1)
    m = r.choice([0, 1, 2, 3])
    slots = bytearray(r.bytes(0x100))
    for k in range(m):
        at = 0x400 + 0x80 * k
        sf = r.choice([FOLDER, FOLDER, texman.STR + at, 0]) if folder is not None else r.choice([0, 0, texman.STR + at])
        text = r.choice([folder or b'', b'', b'a', r.choice(others)])
        strings[at:at + len(text) + 1] = text + bytes(1)
        w = r.choice(words + others[:2]) if words else r.choice(others)
        strings[at + 0x40:at + 0x40 + len(w) + 1] = w + bytes(1)
        put(slots, 0x10 * k, 'III', sf, texman.STR + at + 0x40, TEXS + 0x60 * r.randrange(2))
        put(slots, 0x10 * k + 0xc, 'i', r.choice([1, 2, 0]))
    regions[texman.STR] = bytes(strings)
    regions[texman.MGR] = bytes(slots)
    regions[texman.MGR + 0x10000] = struct.pack('<IiI', m, r.choice([-1, -1] + list(range(m))), r.choice([0, 2, 4, 8]))
    put(regions[0x87a24c], 0x1c, 'I', texman.MGR)   # ([0x87a268], inside texfile.py's 0x87a24c region)
    regions[TEXS] = r.bytes(0x60 * 2)
    info = put(bytearray(r.bytes(0x10)), 4, 'I', FOLDER if folder is not None else 0)
    put(info, 0xc, 'I', NAMES)
    regions[0x879ad0] = put(bytearray(r.bytes(0x204)), 0x200, 'i', r.choice([r.randrange(0x40), 0, 0x3f]))
    regions[0x8bd508] = struct.pack('<I', r.getrandbits(32) & ~0x1000000)
    regions[DEF] = def_bytes(r)
    return regions, info, n


def a_set(r, n, listed):
    s = bytearray(r.bytes(0x10))
    put(s, 0, 'II', DEF, INFO)
    put(s, 8, 'I', listed)
    put(s, 0xc, 'h', r.choice([n, n, n, 0, -1]))
    return s


def load_textures(r):
    regions, info, n = files(r, 1, 0)
    put(info, 0xa, 'H', n)
    regions[INFO] = bytes(info)
    regions[LIST2] = r.bytes(0x10)
    regions[SET2] = a_set(r, n, LIST2)
    return Call(0x575e10, [SET2], regions, conv='t', ret='a')


def load_deferred(r):
    regions, info, n = files(r, 1, 0)
    put(info, 0xa, 'H', n)
    regions[INFO] = bytes(info)
    k = r.choice([0, 1, 2, 0, -1])
    waiting = bytearray(r.bytes(0x404))
    for j in range(max(k, 0)):
        regions[SET2 + 0x40 * j] = a_set(r, n, LIST2 + 0x20 * j)
        regions[LIST2 + 0x20 * j] = r.bytes(0x10)
        put(waiting, 4 * j, 'I', SET2 + 0x40 * j)
    regions[DEFERRED] = put(waiting, 0x400, 'i', k)
    return Call(0x575b90, [], regions, conv='c', ret='v')


def init(r, op):
    blank = r.random() < 0.3
    fail = r.choice([0] * 6 + [1, 2, 3] if blank else [0] * 8 + [1, 2])
    regions, info, n = files(r, 2, fail)
    if blank:
        regions[DEF] = bytes([r.getrandbits(8) | 0x80]) + r.bytes(3)
    elif regions[DEF][0] & 0x80:
        regions[DEF] = bytes([regions[DEF][0] & 0x7f]) + regions[DEF][1:]
    put(info, 0xa, 'H', r.choice([n, n, n, 0, 1]))
    names = bytearray(regions[NAMES])
    if blank:
        put(names, 6, 'H', r.choice([1, 2, 4, 8, 0x10, 0x20, 0x40, 0]))
    regions[NAMES] = bytes(names)
    regions[INFO] = bytes(info)
    again = r.random() < 0.25
    s = bytearray(r.bytes(0x10))
    if again:
        put(s, 8, 'I', LIST2)
        put(s, 0xc, 'h', r.choice([0, 1, 2, -1]))
        regions[LIST2] = struct.pack('<II', TEXS, TEXS + 0x60) + r.bytes(8)
    else:
        put(s, 8, 'I', 0)
    regions[SET2] = s
    put(regions[0x8bd550], 0, 'I', r.getrandbits(32) & ~0x40000000 | (0x40000000 if r.random() < 0.3 else 0))
    regions[DEFERRED] = put(bytearray(r.bytes(0x404)), 0x400, 'i', r.choice([0, 1, 0xff, 0x100, 0x101]))
    if op == 'Pair':
        regions[PAIR] = struct.pack('<II', DEF, INFO)
        return Call(0x575ee0, [SET2, PAIR], regions, conv='t', ret='a')
    return Call(0x575c60, [SET2, DEF, INFO], regions, conv='t', ret='a')


def case(r):
    op = r.deal(OPS)
    if op == 'Release':
        return release(r)
    if op in ('Init', 'Pair'):
        return init(r, op)
    if op == 'LoadTextures':
        return load_textures(r)
    if op == 'LoadDeferred':
        return load_deferred(r)
    mem = bytearray(r.bytes(0x60))
    put(mem, 8, 'I', TEXTURES)
    if op == 'Ctor':
        return Call(0x575bd0, [RAM], {RAM: mem}, conv='t', ret='a')
    i = r.randrange(8) if op == 'Texture' else r.choice([r.randrange(8), -1, r.getrandbits(32)])
    return Call(CALLS[op], [RAM, i], {RAM: mem}, conv='t', ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
