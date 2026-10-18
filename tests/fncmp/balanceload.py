# The game's balance files loaded from the data folder against the originals: GraphicsSettings_Load (0x44dc60: gBalanceGraphics for the flags, then
# the settings applied), Options_CheckGraphicsMode (0x423bc0: a mode's graphics file on the stack against gBalanceGraphics, the options' change level)
# and Options_LoadBalanceFiles (0x45b1f0: gBalanceSound, the options' defaults, gBalanceOnline and the servers, gBalanceResolution when it exists);
# and two callers: GraphicsSettings_LoadForOptions (0x44de20: the game options' mode, the flag words) and Options_NoteChanges (0x423740: options and
# their copy equal or not, two dwords either side of 0x785910).
# The real schemas; each file's values entries over zeroed data. The folder (the wide string at 0xf816c0 + 0x28) with or without its "\", the names
# Strings of a few texts; FindFirstFileA (the size: 0, a few bytes, not found) and GetFileAttributesA recorded; the text whatever the heap held (its
# blocks filled with comment lines, blank lines or 0xcd), the heap failing now and then. The files on the stack (Check, Changes) read and parsed
# (their fields the defaults, 0), the fields of gBalanceGraphics they are checked against mostly 0.
# usage: balanceload.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Graphics,Check,All,ForOptions,Changes)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, IMPORT_TABLE, LOG, Bench, Call, RAM, fill, global_heap, heap, import_table, log_region, put, set_return, strings, vtable  # noqa: E402

GETTERS = [0x402a50, 0x402b10, 0x402b80, 0x402bf0, 0x402c60, 0x402cd0]
B = Bench([(0x44dc60, 0x44dd4e), (0x423bc0, 0x423db5), (0x45b1f0, 0x45b33a), (0x619240, 0x619244), (0x5c53c0, 0x5c5510), (0x424cf0, 0x424dc0),
           (0x401030, 0x401212), (0x401250, 0x40127a), (0x401280, 0x4013dc), (0x4013e0, 0x40161d), (0x401650, 0x4017a0), (0x4017a0, 0x402720),
           (0x402720, 0x402a1e), (0x405710, 0x405742), (0x5f5660, 0x5f5690), (0x5b0d60, 0x5b0d64), (0x412c90, 0x412ca2), (0x4143f0, 0x414402),
           (0x4d1e60, 0x4d1e72), (0x5da3c0, 0x5da3c1), (0x5f56f0, 0x5f5a30), (0x5f5ad0, 0x5f5bc0), (0x5f5c70, 0x5f5d0e), (0x5f5e20, 0x5f5e90),
           (0x5f5fc0, 0x5f7f00), (0x67a400, 0x6902c0), (0x44d8f0, 0x44dc52), (0x583280, 0x5832f0), (0x585710, 0x58571d),
           (0x423690, 0x423714), (0x423dd0, 0x423e89), (0x51b3c0, 0x51b3e0), (0x51bb80, 0x51bc50), (0x45aee0, 0x45b1e4), (0x45b340, 0x45b9a0),
           (0x44de20, 0x44df19), (0x423ba0, 0x423bb1), (0x423740, 0x4237ee)]
          + [(a, a + 0x12) for a in GETTERS])
FILES = {0x785828: (0x6fd628, 15), 0x785868: (0x6fd630, 31), 0x7858e0: (0x6fd638, 3), 0x7858f0: (0x6fd640, 27)}   # (vtable, schema records)
SIZES = {0x785828: 0x40, 0x785868: 0x78, 0x7858e0: 0x10, 0x7858f0: 0x48}
FOLDER, BACKSLASH, OPTIONS = 0xf816c0, 0xf80be0, 0x78d8d8
NAMES = {'Graphics': (0x79e3d8, 0x79e408, 0x79e660), 'Check': (0x78e338, 0x78e368, 0x78e5c0), 'All': (0x7a1c28, 0x7a0c58, 0x7a17c8)}
TEXTS = RAM + 0x7000   # the Strings' texts, 0x40 apart
FIND_FIRST, FIND_CLOSE, ATTRIBUTES, OPEN = 45, 46, 47, 9
VALUES = RAM + 0xc000   # (in the regions' chunk, not in a region: a stack file's values block)
FILE_STREAM_VTABLE, GLOBALS = 0x6fd930, 0xfa6ec8


def string(regions, at, k, text):
    regions[TEXTS + 0x40 * k] = text + b'\0'
    regions[at] = struct.pack('<5I', 0x703090, len(text), TEXTS + 0x40 * k, len(text), 0x10)


def balance(r, regions, at, k, size=None):   # a balance file at `at`: its vtable, the values entries (one per schema record) over zeroed data
    vt, n = FILES[at]
    vals = RAM + 0x1000 + 0x200 * k
    obj = bytearray(size or SIZES[at])
    for j in range(8, len(obj), 4):
        put(obj, j, 'I', r.choice([0, 0, 1, 1, 2, 3, r.getrandbits(32)]))
    put(obj, 0, 'II', vt, vals)
    entries = bytearray(0xc * n)
    for j in range(n):
        put(entries, 0xc * j, 'IiI', RAM + 0x3000 + 0x800 * k + 0x40 * j, 1, RAM + 0x6000 + 0x100 * k + 4 * j)
    regions.update({at: obj, vals: entries, RAM + 0x3000 + 0x800 * k: bytes(0x800), RAM + 0x6000 + 0x100 * k: bytes(0x80)})
    return obj


def common(r, op, stack=False):   # stack: a balance file on the stack, whose values block (its entries the stack's addresses, unlike between the
    regions = {}                     # sides) is the heap's first: the heap starts at VALUES, outside the regions, its later blocks in the region after it
    log = log_region()
    imports = {'FindFirstFileA': (FIND_FIRST, 0x6fd100, 2, 1, 2), 'FindClose': (FIND_CLOSE, 0x6fd104, 1, 0, 0),
               'GetFileAttributesA': (ATTRIBUTES, 0x6fd10c, 1, 1, 0)}
    iat = import_table(log, imports)
    for at, v in global_heap().items():
        iat[at - IMPORT_TABLE:at - IMPORT_TABLE + 4] = v
    regions[IMPORT_TABLE] = iat
    size = r.choice([0, 0, 0, 0, 3, 16, 40, 0xffffffff] if not stack else [3, 16, 40, 40, 0xffffffff])   # (stack: an empty file leaves its fields unset)
    set_return(log, FIND_FIRST, 0xffffffff if size == 0xffffffff else 0x1234)
    fill(log, FIND_FIRST, 1, bytes(0x20) + struct.pack('<I', size) + bytes(0x10), 0)
    fill(log, OPEN, 0, struct.pack('<i', r.choice([0] * 8 + [-1])), 1)
    strings(log, OPEN, 2, 1)
    set_return(log, ATTRIBUTES, r.choice([0x20, 0x20, 0x10, 0xffffffff]))
    regions[FILE_STREAM_VTABLE] = vtable({0: 1, 2: 2, 3: 2, 4: 2, 9: 3})
    regions[GLOBALS] = (b'tp.exe' + bytes(0xfe) + bytes(4) + b'C:\\tpw\\' + bytes(0xfd) + bytes(4) + bytes(0x108))[:0x318]
    regions[LOG] = log
    block = heap(0x8000, fail=r.choice([0] * 16 + [6, 7, 9]))
    line = r.choice([b'# comment line\r\n', b'               \n', None])   # (16 bytes: every block starts a line)
    if line:
        block[0x10:] = line * ((len(block) - 0x10) // 16)
    if stack:   # (the Graphics values block, 31 entries of 12 bytes, at VALUES; the blocks after it from VALUES + 0x180)
        put(block, 0, 'I', VALUES | struct.unpack_from('<I', block, 0)[0] & 15)
        regions[VALUES + 0x180] = block[0x10:0x3e90]
        block = block[:0x10]
    regions[HEAP] = block
    regions[0x785de8] = r.bytes(0x60)
    regions[0xfb1440] = bytes(4)
    folder = r.choice([b'C:\\tpw\\data', b'C:\\tpw\\data\\', b'', b'd'])
    regions[TEXTS + 0x200] = folder + b'\0'
    regions[FOLDER] = bytes(0x28) + struct.pack('<5I', 0x7030c8, len(folder), TEXTS + 0x200, len(folder), 0x10)
    string(regions, BACKSLASH, 9, b'\\')
    for k, at in enumerate(NAMES[op]):
        string(regions, at, k, r.choice([b'Graphics\\High.sam', b'Low.sam', b'Online.sam', b'Sound.sam', b'x', b'']))
    return regions


def graphics(r, options=False):   # GraphicsSettings_Load (cdecl (flags)) or GraphicsSettings_LoadForOptions (cdecl: the game options' mode)
    regions = common(r, 'Graphics')
    balance(r, regions, 0x785868, 0)
    regions.update({0x8bd550: r.bytes(4), 0x7a1a8c: r.bytes(4), 0x79d86c: r.bytes(4), 0x8c162c: r.bytes(4), 0x8bd4c0: r.bytes(4), 0x8bcbc8: r.bytes(4),
                    0x8bcbc8 + 0x8c8: r.bytes(0x18), 0x8c1828 + 0x2ec8: r.bytes(4)})
    if options:
        game = bytearray(r.bytes(0xa0))
        put(game, 0, 'I', r.choice([0, 0, 1, 1, r.getrandbits(32)]))
        put(game, 8, 'I', r.choice([0, 1, 2, 2, 3, r.getrandbits(32)]))
        put(game, 0xc, 'I', r.choice([0, 1, 1, 2]))
        regions.update({OPTIONS: game, 0x8c1620: r.bytes(4), 0x8bd524: r.bytes(4)})
        return Call(0x44de20, [], regions, conv='c', ret='a')
    flags = r.choice([0x800, 0x1000, 0x2000, 0x1800, 0x3000, 0x2800, 0, r.getrandbits(32), 0xffff07ff])
    return Call(0x44dc60, [flags], regions, conv='c', ret='a')


def graphics_fields(r, obj):   # the fields the check compares (+0xc, +0x34, +0x3c, +0x40, +0x44, +0x68) mostly 0 (a parsed file's defaults): one of
    fields = (0xc, 0x34, 0x3c, 0x40, 0x44, 0x68)   # them not, or each now and then
    one = r.choice(fields) if r.random() < 0.5 else None
    for j in fields:
        put(obj, j, 'I', r.choice([1, r.getrandbits(32)]) if j == one or one is None and r.random() < 0.25 else 0)


def check(r):   # Options_CheckGraphicsMode (thiscall (mode)): the options at RAM, their change level +0x94 below 2 or not
    regions = common(r, 'Check', True)
    graphics_fields(r, balance(r, regions, 0x785868, 0))
    options = put(bytearray(r.bytes(0xa0)), 0x94, 'i', r.choice([0, 0, 1, 1, 2, 3, -1, -0x80000000]))
    regions[RAM] = options
    mode = r.choice([0, 1, 2, 0, 1, 2, 3, r.getrandbits(32)])
    return Call(0x423bc0, [RAM, mode], regions, conv='t', ret='b')


def changes(r):   # Options_NoteChanges (thiscall): the options at RAM against their copy (+0x3c..), around the dword 0x785910
    regions = common(r, 'Check', True)
    graphics_fields(r, balance(r, regions, 0x785868, 0))
    options = bytearray(r.bytes(0xa0))
    limit = r.choice([0, 50, 100, -1, r.getrandbits(32)]) & 0xffffffff
    for at in (0, 4, 8, 0xc):
        put(options, at, 'I', r.choice([0, 1, 2, r.getrandbits(32)]))
        if r.random() < 0.6:
            options[at + 0x3c:at + 0x40] = options[at:at + 4]
    for at in (0x30, 0x6c):
        put(options, at, 'I', r.choice([limit, limit - 1, limit + 1, 0, 100, r.getrandbits(32)]) & 0xffffffff)
    put(options, 0x94, 'i', r.choice([0, 0, 1, 1, 2, 3, -1]))
    regions.update({RAM: options, 0x785910: struct.pack('<I', limit)})
    return Call(0x423740, [RAM], regions, conv='t', ret='v')


def load_all(r):   # Options_LoadBalanceFiles (cdecl): the three files, the options (0x78d8d8), the servers (0x7a14f0) and their names
    regions = common(r, 'All')
    online = balance(r, regions, 0x785828, 1)
    for j in range(7):
        put(online, 0x24 + 4 * j, 'I', TEXTS + 0x300)
    regions[TEXTS + 0x300] = b'\0'
    balance(r, regions, 0x7858e0, 2)
    sound = balance(r, regions, 0x7858f0, 3)
    put(sound, 0x34, 'I', r.choice([0, 0, 1, 0x100, r.getrandbits(32)]))
    for j in (0x38, 0x3c, 0x40, 0x44):
        put(sound, j, 'I', r.choice([0, 50, 100, r.randrange(0, 101), r.getrandbits(32)]))
    regions.update({OPTIONS: r.bytes(0x80), 0x803ab0: r.bytes(0x40), 0x802ba0: r.bytes(0x10), 0x7a14f0: bytes(0x7c)})
    return Call(0x45b1f0, [], regions, conv='c', ret='v')


def case(r):
    op = r.deal(OPS)
    if op == 'Graphics':
        return graphics(r)
    if op == 'Check':
        return check(r)
    if op == 'ForOptions':
        return graphics(r, True)
    if op == 'Changes':
        return changes(r)
    return load_all(r)


OPS = [k for k in os.environ.get('OPS', 'Graphics,Check,All,ForOptions,Changes').split(',') if k]

if __name__ == '__main__':
    B.main(case)
