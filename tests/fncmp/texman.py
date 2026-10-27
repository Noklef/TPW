# The texture manager's load flags setter and its release (src/game/gfx/texman: 0x575b80, 0x574b10) against the originals: any
# flags over any old ones; for the release the manager at 0x87a268 or none, 0-3 slots in use (or a negative count) whose texture
# arrays (as texturemem.py) are set or not; for a texture's release 0-4 slots (the count theirs, one less, 0 or negative), their arrays
# set, 0 or any bits, their users 1, more, 0 or negative, the array asked for one of theirs or another (src/game/gfx/texman: 0x575ad0).
# A texture's load (0x5755f0): the load flags (not 1) and texfile.py's scene for them, the textures' block failing now and then; its
# lookup (0x575870): 0-4 slots (or 0x1000 and more), the folder and name found among them (pointers or equal strings) or loaded from
# texfile.py's name split into [0x786de4], the folder and the name (at a backslash, or anywhere), the fallback slot or none.
# The landscape's texture added (src/game/terrain/landscape: 0x5850c0): its count 0-0x10 or more (or negative), the slot's old array
# (plain textures) set or not, the load's scene as above with the manager at [0x87a268]. The same scene for the weather renderer's
# texture (src/game/render/weatherfx: 0x57d7f0; slot 1-3 or a pointer, set already or not) and the sprite queue's (src/game/render/
# spritequeue: 0x58a5c0; its old array plain textures or none). The landscape's sky (src/game/terrain/landscape: 0x5852b0): as
# weather_load_all()'s three loads through Quickload_Locate, a folder name of 0-40 characters, the old arrays at +0x1fd0..+0x1fd8 plain
# textures or none, the second's colours sampled (its file a TGA). The weather renderer's reload (src/game/render/weatherfx: 0x57d850):
# weather_load_all()'s scene on the renderer itself, its +4 any. The static textures' exit handler (src/game/render/spritequeue: 0x58ac10):
# the record at 0x8c47b0 with plain_array()'s textures at +8 (or none) and a block at +0xc in the tracked table (as it is, 4 above, or
# not) or 0.
# usage: texman.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=SetLoadFlags,Destroy,Release,Load,Acquire,LandAdd,WeatherLoad,QueueLoad,WeatherLoadAll,LandSky,WeatherReload,StaticFree)
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

CALLS = {'SetLoadFlags': 0x575b80, 'Destroy': 0x574b10, 'Release': 0x575ad0, 'Load': 0x5755f0, 'Acquire': 0x575870, 'LandAdd': 0x5850c0, 'WeatherLoad': 0x57d7f0, 'QueueLoad': 0x58a5c0,
         'WeatherLoadAll': 0x5508c0, 'LandSky': 0x5852b0, 'WeatherReload': 0x57d850, 'StaticFree': 0x58ac10}
RANGES = wctimage.merged(texfile.RANGES + [(0x575b80, 0x575b8d), (0x575ad0, 0x575b7a), (0x574b10, 0x574bc6), (0x590a70, 0x590a75), (0x590b90, 0x590c39),
                                             (0x579b40, 0x579c13), (0x58dc50, 0x58dd5e), (0x67afa0, 0x67b0d0), (0x67acb0, 0x67acb1), (0x5755f0, 0x575acb),
                                             (0x590a00, 0x590a70), (0x590d40, 0x590f7e), (0x5850c0, 0x5851c6), (0x57d7f0, 0x57d849), (0x58a5c0, 0x58a630),
                                             (0x56af20, 0x56af9c), (0x5508c0, 0x550927), (0x5852b0, 0x58568c), (0x585ce0, 0x585e1e),
                                             (0x57d850, 0x57d909), (0x58ac10, 0x58ac6e)])
B = Bench(RANGES)
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


LAND, OUT = RAM + 0xa0000, RAM + 0xa3000   # (a landscape: 0x10 texture entries of 0x20 at +0x1fdc, their count at +0x21dc)
WEATHER, QUEUE = RAM + 0xa4000, RAM + 0xa8000   # (only their slots: +0x3348..+0x3357, +0x2400)


def plain_array(r, regions):
    """1 or 2 plain textures at ARRAYS + 4 (texturemem.py's regions would overlap vfs.py's) half the time, else 0; returns it."""
    if r.random() < 0.5:
        return 0
    n = r.choice([1, 2])
    tex = bytearray(r.bytes(0x60 * n))
    for k in range(n):
        put(tex, 0x60 * k + 8, 'I', r.getrandbits(32) & ~0x10801000)
        put(tex, 0x60 * k + 0x3c, 'II', 0, 0)
    regions[ARRAYS] = struct.pack('<I', n) + bytes(tex)
    return ARRAYS + 4


def scene_at_manager(r):
    regions = load_scene(r)
    regions[0x87a268] = struct.pack('<I', MGR)
    return regions


def weather_load(r):
    regions = scene_at_manager(r)
    slots = bytearray(r.bytes(0x10))
    for k in range(4):
        put(slots, 4 * k, 'I', r.choice([0, 0, 0, r.getrandbits(32)]))
    regions[WEATHER + 0x3348] = slots
    which = r.choice([1, 2, 3, 1, 2, 3, WEATHER + 0x3354])
    return Call(0x57d7f0, [WEATHER, which, vfs.NAME], regions, conv='t', ret='a')


DIRS = RAM + 0xb0000   # (Quickload_Locate's directories, [0x79cb4c])


def weather_load_all(r):
    """0x5508c0: texfile.py's scene for flags 0 as texset.py's (the heap not failing, room for three loads; the memory source over three
    texfile.exact() files, a load after a load reading on), the manager at [0x87a268] with load flags 0, 2, 4 or 8, the renderer at
    [0x8bcbac] (or none, now and then) with weather_load()'s slots; its three loads take the names through Quickload_Locate(name, 1, 0):
    copied to 0x79c9d0 (up to the directories' pointer [0x79cb4c], read too) and kept if the mounts open them (the memory source opens
    any), quickload's loaded flag [0x7a445c] any (flag 1 never searches its tables)."""
    regions = texfile.scene(r, 0)
    regions[HEAP] = heap(0x40000)
    vfs.memory_source(r, regions[LOG], regions, b''.join(texfile.exact(r) for _ in range(3)))
    if r.random() < 0.2:   # (the tracked blocks full: each load's pixels not got, TexManager_Load 0, so the last load answers 0)
        regions[wctimage.COUNT] = struct.pack('<I', 0x8000)
    regions[MGR + 0x10008] = struct.pack('<I', r.choice([0, 2, 4, 8]))
    regions[0x87a268] = struct.pack('<I', MGR)
    slots = bytearray(r.bytes(0x10))
    for k in range(4):
        put(slots, 4 * k, 'I', r.choice([0, 0, 0, r.getrandbits(32)]))
    regions[WEATHER + 0x3348] = slots
    regions[0x8bcbac] = struct.pack('<I', WEATHER if r.random() < 0.9 else 0)
    regions[0x79c9d0] = r.bytes(0x17c)
    regions[0x79cb4c] = struct.pack('<I', DIRS)
    regions[DIRS] = r.bytes(0x100)
    regions[0x7a445c] = r.bytes(4)
    return Call(0x5508c0, [], regions, ret='a')



def weather_reload(r):
    """0x57d850 (thiscall on the renderer): weather_load_all()'s scene, the renderer's +4 any."""
    regions = weather_load_all(r).regions
    regions[WEATHER] = r.bytes(8)
    return Call(0x57d850, [WEATHER], regions, conv='t', ret='a')


def static_free(r):
    regions = {HEAP: heap(), **global_heap(), LOG: log_region()}
    textures = plain_array(r, regions)
    block = r.choice([0, HEAP + 0x200, HEAP + 0x200, r.getrandbits(32)])
    table = bytearray(8 * 8)
    for k in range(8):
        put(table, 8 * k, 'II', r.getrandbits(32), r.choice([block, (block + 4) & 0xffffffff, r.getrandbits(32)]))
    regions[texturemem.PAIRS] = table
    regions[texturemem.COUNT] = struct.pack('<I', r.choice([0, 1, 2, 4, 8]))
    regions[0x8c47b0] = put(bytearray(r.bytes(0x18)), 8, 'II', textures, block)
    return Call(0x58ac10, [], regions)


def land_sky(r):
    """0x5852b0: weather_load_all()'s scene (its three files, the second a TGA; the loads failing now and then), the folder a
    name of 0-40 characters, the landscape's old arrays plain textures (1 or 2) or none."""
    regions = texfile.scene(r, 0)
    regions[HEAP] = heap(0x40000)
    files = [texfile.exact(r) for _ in range(3)]
    while files[1][0] != 0:   # (the colours' texture a TGA: a WCT image's pixels are its surface object's, locked through its vtable)
        files[1] = texfile.exact(r)
    vfs.memory_source(r, regions[LOG], regions, b''.join(files))
    if r.random() < 0.3:   # (the tracked blocks full, or two or three from full: the later loads' pixels not got, TexManager_Load 0)
        regions[wctimage.COUNT] = struct.pack('<I', r.choice([0x8000, 0x7ffe, 0x7ffd]))   # (0x7fff: crashes in both a third of the time)
    regions[MGR + 0x10008] = struct.pack('<I', r.choice([0, 2, 4, 8]))
    regions[0x87a268] = struct.pack('<I', MGR)
    regions[0x79c9d0] = r.bytes(0x17c)
    regions[0x79cb4c] = struct.pack('<I', DIRS)
    regions[DIRS] = r.bytes(0x100)
    regions[0x7a445c] = r.bytes(4)
    regions[0x879ad0] = put(bytearray(r.bytes(0x204)), 0x200, 'i', r.choice([r.randrange(0x40), 0, 0x3e, 0x3f, 0x40]))
    regions[0x8798c8] = bytes(4)
    land = bytearray(r.bytes(0x1fe0))
    for k in range(3):
        array = 0
        if r.random() < 0.5:
            n = r.choice([1, 2])
            tex = bytearray(r.bytes(0x60 * n))
            for j in range(n):
                put(tex, 0x60 * j + 8, 'I', r.getrandbits(32) & ~0x10801000)
                put(tex, 0x60 * j + 0x3c, 'II', 0, 0)
            regions[ARRAYS + 0x200 * k] = struct.pack('<I', n) + bytes(tex)
            array = ARRAYS + 0x200 * k + 4
        put(land, 0x1fd0 + 4 * k, 'I', array)
    regions[LAND] = bytes(land)
    name = bytes(r.choice(b'abcdefghijklmnopqrstuvwxyz0123456789_' + bytes([92])) for _ in range(r.choice([0, 1, 5, 12, 40, r.randrange(41)])))
    regions[OUT] = name + bytes(1)
    return Call(0x5852b0, [LAND, OUT], regions, conv='t', ret='a')


def queue_load(r):
    regions = scene_at_manager(r)
    regions[QUEUE + 0x2400] = struct.pack('<I', plain_array(r, regions)) + r.bytes(4)
    return Call(0x58a5c0, [QUEUE, vfs.NAME], regions, conv='t', ret='a')


def land_add(r):
    regions = scene_at_manager(r)
    regions[0x879ad0] = put(bytearray(r.bytes(0x204)), 0x200, 'i', r.choice([r.randrange(0x40), 0, 0x3f, 0x40]))
    regions[0x8798c8] = bytes(4)
    count = r.choice([0, 1, 5, 0xe, 0xf, 0xf, 0x10, 0x11, -1])
    land = put(bytearray(r.bytes(0x21e0)), 0x21dc, 'i', count)
    if count < 0x10:   # (-1: the entry below the first, sic)
        put(land, 0x1fdc + 0x20 * count, 'I', plain_array(r, regions))
    regions[LAND] = bytes(land)
    regions[OUT] = r.bytes(4)
    args = [OUT, vfs.NAME] + [r.getrandbits(32) for _ in range(6)]
    return Call(0x5850c0, [LAND] + args, regions, conv='t', ret='a')


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
    if op == 'LandAdd':
        return land_add(r)
    if op == 'WeatherLoad':
        return weather_load(r)
    if op == 'QueueLoad':
        return queue_load(r)
    if op == 'LandSky':
        return land_sky(r)
    if op == 'WeatherLoadAll':
        return weather_load_all(r)
    if op == 'WeatherReload':
        return weather_reload(r)
    if op == 'StaticFree':
        return static_free(r)
    if op == 'Destroy':
        return destroy(r)
    if op == 'Release':
        return release(r)
    regions = {RAM + 0x10000: r.bytes(0x10)}
    return Call(0x575b80, [RAM, r.choice([0, 2, 4, 8, r.getrandbits(32)])], regions, conv='t')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
