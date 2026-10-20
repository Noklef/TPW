# The virtual file system's paths (src/game/files/vfs: 0x41cff0) against the originals. StripBase: a base folder (now and then ending in
# the archive pattern) and a path under it or not (the base's case varied, cut short or other text; leading ".", "\" runs), out a
# string of any text; the globals "\", ".", ".wad*\" (0x78bf40), ".wad*" (0x78c810), "" (0x78be50) and the folder code's "\" (0xf80be0),
# now and then other texts, so that the archive branch, dead with the game's own pair, runs too ("." and the tail not empty, an archive in
# a folder: as the game's, else both fault). The strings as gamestring.py's, their blocks from the heap region.
# Open (0x5c4d50): a file opened through the mounts' current source and its archive, or the list of the others (see open_()); Exists
# (0x46f120) the same, the stream got closed and deleted. Locate (0x44a280): a ride file through the quickload indexes or Exists (see
# locate()).
# usage: vfs.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=StripBase,Open,Exists,Locate)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import LOG, Bench, Call, HEAP, RAM, STUBS, SEEK, blob, echo, fill, heap, log_region, put, recorder, set_return, strings  # noqa: E402
import files  # noqa: E402
import gamestring  # noqa: E402
from gamestring import BACKSLASH, DOT, LOCALE, SLASH2, string  # noqa: E402

RANGES = gamestring.B_RANGES + [(0x41cff0, 0x41d2d0), (0x41ce90, 0x41ce94), (0x421720, 0x421780), (0x4217e0, 0x421800), (0x5c4d50, 0x5c4f70),
                                 (0x46f120, 0x46f160), (0x5c4f80, 0x5c4fb0), (0x618d30, 0x618d90), (0x44a280, 0x44a3c4), (0x461790, 0x461a60),
                                 (0x45b9d0, 0x45ba15),
                                 (0x4036a0, 0x4036e0)]   # (the last: StreamBase's read and write, for memory_source())
B = Bench(RANGES)
TAIL, WAD, EMPTY = 0x78bf40, 0x78c810, 0x78be50
PATH, BASE, OUT = RAM, RAM + 0x40, RAM + 0x80
WORDS = [b'data', b'Data', b'ui', b'x.wct', b'a', b'TP', b'levels']


def flip(r, text, p=0.3):
    return bytes(c ^ 0x20 if (0x61 <= c <= 0x7a or 0x41 <= c <= 0x5a) and r.random() < p else c for c in text)


def folder(r):
    return b'\\'.join(r.choice(WORDS) for _ in range(r.choice([0, 1, 1, 2, 3])))


def strip_base(r):
    regions = {HEAP: heap(), LOCALE: bytes(4)}
    base = folder(r)
    archive = r.random() < 0.3
    if archive:
        base = (base or b'data') + r.choice([b'.wad*', b'.WAD*', b'.wad*\\', b'.wad'])
    elif r.random() < 0.4:
        base += b'\\'
    k = r.random()
    if k < 0.6:   # under the base (as the archive's folder: its pattern dropped, a "\\" after it)
        head = base[:-len(b'.wad*')] + b'\\' if archive and base.endswith(b'.wad*') else base
        path = flip(r, head) + r.choice([b'', b'\\', b'\\\\', b'/']) + folder(r)
    elif k < 0.8:
        path = flip(r, base[:r.randrange(len(base) + 1)])
    else:
        path = folder(r)
    if r.random() < 0.3:
        path = r.choice([b'.', b'.\\', b'..\\', b'\\', b'\\\\', b'.\\\\']) + path
    if r.random() < 0.2:
        base = r.choice([b'.\\', b'\\', b'.']) + base
    string(r, 0, PATH, RAM + 0x1000, regions, path)
    string(r, 0, BASE, RAM + 0x1400, regions, base)
    string(r, 0, OUT, RAM + 0x1800, regions)
    globals_(r, regions)
    return Call(0x41cff0, [PATH, BASE, OUT], regions, ret='a')


def globals_(r, regions):
    for at, data, usual, other in ((BACKSLASH, RAM + 0x2000, b'\\', [b'/', b'\\\\', b'a']), (DOT, RAM + 0x2100, b'.', [b'.\\', b'..']),
                                    (TAIL, RAM + 0x2200, b'.wad*\\', [b'.wad*', b'.wad*', b'.wad']),
                                    (WAD, RAM + 0x2300, b'.wad*', [b'.wad*\\', b'.wad', b'']), (EMPTY, RAM + 0x2400, b'', [b'x', b'\\a']),
                                    (SLASH2, RAM + 0x2500, b'\\', [b'/', b'\\\\'])):
        string(r, 0, at, data, regions, usual if r.random() < 0.7 else r.choice(other))


MOUNTS, SOURCE, ARCHIVE, LIST, NODES, OBJS, NAME = RAM + 0x3000, RAM + 0x3100, RAM + 0x3200, RAM + 0x3300, RAM + 0x3400, RAM + 0x3500, RAM + 0x3800
VT_SOURCE, VT_ARCHIVE, VT_OBJ = RAM + 0x3900, RAM + 0x3980, RAM + 0x3a00


def open_(r, address=0x5c4d50, extra=()):
    """0x5c4d50 (cdecl; name): mounts()'s case and a name under its root or not (strip_base()'s texts)."""
    log = log_region()
    regions = {LOG: log, HEAP: heap(fail=r.choice([0] * 7 + [1])), LOCALE: bytes(4)}
    root = mounts(r, log, regions)
    regions[NAME] = name_under(r, root) + bytes(1) + r.bytes(7)
    return Call(address, [NAME, *extra], regions, ret='a')


def name_under(r, root):
    k = r.random()
    if k < 0.6:
        name = flip(r, root) + r.choice([b'\\', b'', b'\\\\']) + folder(r)
    elif k < 0.75:
        name = flip(r, root[:r.randrange(len(root) + 1)])
    else:
        name = folder(r) + r.choice([b'', b'.wct'])
    if r.random() < 0.2:
        name = r.choice([b'.\\', b'\\']) + name
    return name


def mounts(r, log, regions):
    """The mounts object [0x786b9c] with a list of 0-3 objects (slot 8: k 7) or none and a source (slot 8: k 2)
    or none; the source's root at +0x18 a narrow String mostly (the code reads it as a WString: AssignWide copies its bytes), else a
    wide one, and its +0x38 an archive (slot 0xc: k 3, the result struct filled through its first argument and returned, its dword any
    sign) or 0; the answers 1 mostly, else 0 or another; the stream's block failing now and then (the caller's heap()). The String
    arguments logged as their dwords (stack addresses), the relative path as its text. Returns the root's text."""
    globals_(r, regions)
    root = folder(r) or b'data'
    if r.random() < 0.3:
        root += b'\\'
    src = bytearray(r.bytes(0x40))
    put(src, 0, 'I', VT_SOURCE)
    part = {}
    wide = r.random() < 0.2
    text = root if not wide else b''.join(bytes([c, 0]) for c in root)
    string(r, 0, RAM + 0x10, RAM + 0x3c00, part, text)
    head = bytearray(part[RAM + 0x10][:0x14])
    if wide:
        put(head, 0, 'II', 0x7030c8, len(root))
    src[0x18:0x2c] = head
    regions[RAM + 0x3c00] = part[RAM + 0x3c00] + r.bytes(0x20)
    put(src, 0x38, 'I', ARCHIVE if r.random() < 0.75 else 0)
    regions[SOURCE] = src
    regions[ARCHIVE] = put(bytearray(r.bytes(0x20)), 0, 'I', VT_ARCHIVE)
    vt = bytearray(r.bytes(0x20))
    put(vt, 8, 'I', recorder(2, 3))
    regions[VT_SOURCE] = vt
    vt = bytearray(r.bytes(0x20))
    put(vt, 0xc, 'I', recorder(3, 5))
    regions[VT_ARCHIVE] = vt
    vt = bytearray(r.bytes(0x20))
    put(vt, 8, 'I', recorder(7, 3))
    regions[VT_OBJ] = vt
    n = r.choice([0, 1, 2, 3])
    nodes = bytearray(r.bytes(0x40))
    for i in range(n):
        put(nodes, 0x10 * i, 'II', OBJS + 0x20 * i, NODES + 0x10 * (i + 1) if i + 1 < n else 0)
        regions[OBJS + 0x20 * i] = put(bytearray(r.bytes(0x20)), 0, 'I', VT_OBJ)
    regions[NODES] = nodes
    regions[LIST] = put(bytearray(r.bytes(0x10)), 4, 'I', NODES if n else 0)
    regions[MOUNTS] = struct.pack('<II', LIST if r.random() < 0.9 else 0, SOURCE if r.random() < 0.85 else 0)
    regions[0x786b9c] = struct.pack('<I', MOUNTS)
    echo(log, 3, 0)
    fill(log, 3, 0, struct.pack('<i', r.choice([0, 1, 5, -1, -0x80000000, r.getrandbits(31)])))
    strings(log, 3, 1 << 2, out=1)
    blob(log, 2, 1, 0x1f)
    blob(log, 7, 1, 0x1f)
    for k in (2, 7):
        set_return(log, k, r.choice([1, 1, 0, 0, 2, r.getrandbits(32)]))
    return root


VT_MEM, MEMSRC, MEMDATA = RAM + 0x3a80, RAM + 0x3ac0, RAM + 0x70000


def memory_source(r, log, regions, data):
    """After mounts(): the source (always there, without an archive) opens every file (k 2 answers 1) and attaches to the stream a memory
    source over data (at MEMDATA, 0x100 random bytes after it): the stream's vtable 0x703100 and the source written through the open's first
    argument (the second fill). The source is fncmp's memory_stream's layout (vtable, cursor, bytes left, start, size) with the RefObject's
    slots: +4 read, +8 write, +0xc seek (the stubs), +0x10 size and +0x14 position recorders (k 8, 9), +0 and +0x24 (only reached when a
    Release takes +8 to 0) recorder k 10. The close's Release goes through InterlockedDecrement's import, a stdcall recorder (k 11)
    answering 1: the source is never freed."""
    regions[SOURCE] = put(bytearray(regions[SOURCE]), 0x38, 'I', 0)
    regions[MOUNTS] = put(bytearray(regions[MOUNTS]), 4, 'I', SOURCE)
    set_return(log, 2, 1)
    fill(log, 2, 0, struct.pack('<2I', 0x703100, MEMSRC), which=1)
    size = len(data) + 0x100
    regions[MEMSRC] = struct.pack('<5I', VT_MEM, MEMDATA, size, MEMDATA, size)
    vt = bytearray(r.bytes(0x28))
    put(vt, 0, '6I', recorder(10, 1), STUBS, STUBS + 0x10, SEEK, recorder(8, 0), recorder(9, 0))
    put(vt, 0x24, 'I', recorder(10, 0))
    regions[VT_MEM] = vt
    regions[MEMDATA] = bytes(data) + r.bytes(0x100)
    for k in (8, 9):
        set_return(log, k, r.choice([0, len(data), r.getrandbits(32)]))
    regions[0x6fd12c] = struct.pack('<I', recorder(11, 1, True))   # InterlockedDecrement: 1 (the close's Release frees nothing)
    set_return(log, 11, 1)


def exists(r):
    """0x46f120 (cdecl; name, 0x74cfb8): open_()'s case; a stream got is finished and deleted (the game's StreamBase code)."""
    return open_(r, 0x46f120, [0x74cfb8])


def locate(r):
    """0x44a280 (cdecl; name or 0, flags, &index or 0): mounts()'s case and files.py's quickload tables (at RAM + 0x40000 on, loaded or
    not); the name one of files.py's names, now and then under a folder, or one under the root (name_under()); flags of bits 0-3 or any;
    the directories [0x79cb4c] 64 dwords, the buffer 0x79c9d0 up to them."""
    log = log_region()
    regions = {LOG: log, HEAP: heap(fail=r.choice([0] * 7 + [1])), LOCALE: bytes(4)}
    root = mounts(r, log, regions)
    quick, _count = files.quickload(r, RAM + 0x40000)
    regions.update(quick)
    k = r.random()
    if k < 0.4:
        name = files.name(r)
    elif k < 0.6:
        name = folder(r) + b'\\' + files.name(r)
    else:
        name = name_under(r, root)
    regions[RAM + 0x60000] = name + bytes(1) + r.bytes(7)
    regions[0x79c9d0] = r.bytes(0x17c)
    regions[0x79cb4c] = struct.pack('<I', RAM + 0x3d00)
    regions[RAM + 0x3d00] = r.bytes(0x100)
    regions[RAM + 0x3e00] = r.bytes(4)
    flags = r.choice([0, 1, 2, 3, 4, 6, 7, 0xa, 0xb, 0xe, 0xf, 0xc, r.getrandbits(4), r.getrandbits(32)])
    args = [0 if r.random() < 0.04 else RAM + 0x60000, flags, r.choice([RAM + 0x3e00] * 4 + [0])]
    return Call(0x44a280, args, regions, ret='a')


OPS_ALL = {'StripBase': strip_base, 'Open': open_, 'Exists': exists, 'Locate': locate}
OPS = [k for k in os.environ.get('OPS', ','.join(OPS_ALL)).split(',') if k]


def case(r):
    return OPS_ALL[r.deal(OPS)](r)


if __name__ == '__main__':
    B.main(case)
