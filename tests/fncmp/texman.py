# The texture manager's load flags setter and its release (src/game/gfx/texman: 0x575b80, 0x574b10) against the originals: any
# flags over any old ones; for the release the manager at 0x87a268 or none, 0-3 slots in use (or a negative count) whose texture
# arrays (as texturemem.py) are set or not; for a texture's release 0-4 slots (the count theirs, one less, 0 or negative), their arrays
# set, 0 or any bits, their users 1, more, 0 or negative, the array asked for one of theirs or another (src/game/gfx/texman: 0x575ad0).
# A texture's load (0x5755f0): the load flags (not 1) and texfile.py's scene for them, the textures' block failing now and then; its
# lookup (0x575870): 0-4 slots (or 0x1000 and more), the folder and name found among them (pointers or equal strings) or loaded from
# texfile.py's name split into [0x786de4], the folder and the name (at a backslash, or anywhere), the fallback slot or none.
# usage: texman.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=SetLoadFlags,Destroy,Release,Load,Acquire)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import texfile  # noqa: E402
import texturemem  # noqa: E402
import vfs  # noqa: E402
import wctimage  # noqa: E402
from fncmp import Bench, Call, HEAP, LOG, RAM, global_heap, heap, log_region, put  # noqa: E402

CALLS = {'SetLoadFlags': 0x575b80, 'Destroy': 0x574b10, 'Release': 0x575ad0, 'Load': 0x5755f0, 'Acquire': 0x575870}
B = Bench(wctimage.merged(texfile.RANGES + [(0x575b80, 0x575b8d), (0x575ad0, 0x575b7a), (0x574b10, 0x574bc6), (0x590a70, 0x590a75), (0x590b90, 0x590c39),
                                             (0x579b40, 0x579c13), (0x58dc50, 0x58dd5e), (0x67afa0, 0x67b0d0), (0x67acb0, 0x67acb1), (0x5755f0, 0x575acb),
                                             (0x590a00, 0x590a70), (0x590d40, 0x590f7e)]))
MANAGER, ARRAYS = RAM + 0x10000, RAM + 0x5000   # (texturemem.py's regions below RAM + 0x4000)


def destroy(r):
    regions = {HEAP: heap(), **global_heap(), LOG: log_region()}
    manager = bytearray(r.bytes(0x1000c))
    n = r.choice([0, 1, 2, 3])
    put(manager, 0x10000, 'I', r.choice([n, n, n, 0, 0xffffffff]) if n else r.choice([0, 0, 0xffffffff]))
    for k in range(n):
        if r.random() < 0.8:
            m = r.choice([0, 1, 2, 3])
            base = ARRAYS + 0x400 * k
            tex = texturemem.textures(r, regions, base + 4, m + 1)
            regions[base] = struct.pack('<I', m) + tex
            put(manager, 8 + 0x10 * k, 'I', base + 4)
        else:
            put(manager, 8 + 0x10 * k, 'I', 0)
    regions[MANAGER] = manager
    regions[0x87a268] = struct.pack('<I', r.choice([MANAGER, MANAGER, MANAGER, 0]))
    return Call(0x574b10, [], regions, conv='c', ret='a')


def release(r):
    regions = {HEAP: heap(), **global_heap(), LOG: log_region()}
    n = r.choice([0, 1, 2, 3, 4])
    slots = bytearray(r.bytes(0x10 * n + 0x10))
    arrays = []
    for k in range(n):
        if r.random() < 0.85:
            m = r.choice([1, 2, 3])
            base = ARRAYS + 0x400 * k
            regions[base] = struct.pack('<I', m) + texturemem.textures(r, regions, base + 4, m)
            arrays.append(base + 4)
            put(slots, 0x10 * k + 8, 'I', base + 4)
        else:
            put(slots, 0x10 * k + 8, 'I', r.choice([0, r.getrandbits(32)]))
        put(slots, 0x10 * k + 0xc, 'I', r.choice([1, 1, 1, 2, 3, 0, 0xffffffff]))
    regions[MANAGER] = slots
    regions[MANAGER - 0x10] = r.bytes(12) + struct.pack('<I', r.choice([1, 1, 2]))   # (a slot before the first: the search must stop at 0)
    regions[MANAGER + 0x10000] = struct.pack('<III', r.choice([n, n, n, max(n - 1, 0), 0, 0xffffffff]), r.getrandbits(32), r.getrandbits(32))
    arg = r.choice(arrays) if arrays and r.random() < 0.8 else r.choice([ARRAYS + 0x2004, r.getrandbits(32)])
    return Call(0x575ad0, [MANAGER, arg], regions, conv='t', ret='a')


MGR, STR = RAM + 0x80000, RAM + 0x92000   # (the load's manager and strings: clear of texfile.py's regions)


def load_scene(r):
    """The load flags (+0x10008) and texfile.py's scene for them (the textures' block the heap's first allocation, which fails now and
    then); returns the regions with the manager at MGR."""
    flags = r.choice([0, 2, 2, 4, 8, 6, 0xe, 0xa, r.getrandbits(32) & ~1])   # (not 1: the raw read leaves the width and height unset)
    regions = texfile.scene(r, flags, first=1)
    if r.random() < 0.05:
        regions[HEAP] = heap(0x10000, fail=1)
    regions[MGR + 0x10008] = struct.pack('<I', flags)
    return regions


def load(r):
    regions = load_scene(r)
    return Call(0x5755f0, [MGR, vfs.NAME], regions, conv='t', ret='a')


def acquire(r):
    """0-4 slots (or 0x1000 and more, zero), the folder and name looked up among them (the same pointers, equal strings, others; a null
    folder), else loaded: [0x786de4] + folder + name split from texfile.py's name (at a backslash, so the path is the file's, or anywhere);
    the fallback slot -1 or one of them."""
    regions = load_scene(r)
    full = regions[vfs.NAME].split(bytes(1))[0]
    cuts = [k for k in range(len(full)) if full[k:k + 1] == b'\\']
    i = r.randrange(len(full) + 1)
    base, rest = full[:i], full[i:]
    folder = None
    if r.random() < 0.7:
        j = r.randrange(len(rest) + 1)
        k = [c - i for c in cuts if c >= i]
        if k and r.random() < 0.7:   # (the backslash added back)
            j = r.choice(k)
            folder, name = rest[:j], rest[j + 1:]
        else:
            folder, name = rest[:j], rest[j:]
    else:
        name = rest
    regions[0x786de4] = base + bytes(1)
    strings = bytearray(0x800)
    FOLDER, NAME = STR, STR + 0x200
    strings[0:len(folder or b'') + 1] = (folder or b'') + bytes(1)
    strings[0x200:0x200 + len(name) + 1] = name + bytes(1)
    others = [b'', b'a', b'tex', (folder or b'x') + b'y', name[:-1], name + b'z']
    n = r.choice([0, 1, 2, 3, 4])
    huge = r.random() < 0.03
    slots = bytearray(0x10 * 8) if huge else bytearray(r.bytes(0x10 * 8))
    for k in range(0 if huge else n):
        at = STR + 0x400 + 0x80 * k
        sf = r.choice([FOLDER, FOLDER, at, 0]) if folder is not None else r.choice([0, 0, at])
        sn = r.choice([NAME, at + 0x40, at + 0x40])
        text = r.choice([folder or b'', folder or b'', r.choice(others)])
        strings[at - STR:at - STR + len(text) + 1] = text + bytes(1)
        word = r.choice([name, name, r.choice(others)])
        strings[at - STR + 0x40:at - STR + 0x40 + len(word) + 1] = word + bytes(1)
        put(slots, 0x10 * k, 'III', sf, sn, r.getrandbits(32))
        put(slots, 0x10 * k + 0xc, 'i', r.choice([1, 2, 0, -1]))
    regions[STR] = bytes(strings)
    regions[MGR] = bytes(slots)
    count = r.choice([0x1000, 0x1001]) if huge else n
    fallback = r.choice([-1, -1] + list(range(n)) if n else [-1])
    regions[MGR + 0x10000] = struct.pack('<Ii', count, fallback) + regions.pop(MGR + 0x10008)
    if huge:
        regions[MGR + 0x80] = bytes(0x10000 - 0x80)   # (all slots null: the folder set never matches)
        folder = folder if folder is not None else b''
    return Call(0x575870, [MGR, FOLDER if folder is not None else 0, NAME], regions, conv='t', ret='a')


def case(r):
    op = r.deal(OPS)
    if op == 'Load':
        return load(r)
    if op == 'Acquire':
        return acquire(r)
    if op == 'Destroy':
        return destroy(r)
    if op == 'Release':
        return release(r)
    regions = {RAM + 0x10000: r.bytes(0x10)}
    return Call(0x575b80, [RAM, r.choice([0, 2, 4, 8, r.getrandbits(32)])], regions, conv='t')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
