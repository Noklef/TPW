# The named objects registry of src/game/core/namedobjects against the original: the removal of the 14 named globals (0x408f00) from a registry ({list,
# cached} at [0x786b9c]) of 0 to 6 objects named after them (case varied) or not, duplicates included, or without a list; the objects are deleted through
# a recorder (slot 0), NamedList_Find 0x421770, ListRef_Remove 0x421690 and the name comparison run (the STL lock left out as in vobj3.py).
# Also the registry owner's constructor 0x407b90 (the name assigned from a WString at 0x7861c0, allocations failing or not), destructor
# 0x407c90 and its at-exit one 0x407b80 (an owned list of 0 to 5 nodes, the count short of it or not, objects deleted through a recorder's
# slot 3, a registry of 0 to 3 drained through ListRef_DrainAndDelete or none, frees hashed by heap()).
# And its sorted add 0x409620: a name (a String) into an owned list of 0 to 5 named Strings (sorted or not, near misses, the count short of it
# or not, now and then holding the block the new String will get, now and then a count with no head), allocations failing now and then.
# And the owner's frame update 0x409260 (the game clock's step and the flags 0x7a1a14), and its theme labels 0x409a30 / 0x4099b0 (see theme()).
# And its folders 0x4088a0 and 0x4081e0 (see mounts(): the whole chain below runs, the constructors, AddFolderAndWad with fileopen.py's imports, the adds).
# usage: namedobjects.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Remove,Ctor,Dtor,Static,Add,Flags,Theme,ThemeAnsi,Folders,Listed)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import fileopen  # noqa: E402
from fncmp import HEAP, IMPORT_TABLE, LOG, Bench, Call, RAM, fill, global_heap, heap, import_table, log_region, put, set_return, vtable  # noqa: E402


def lock(e):  # the STL lock (0x691de0, thiscall, returns its argument; 0x691ea0 releases): the port leaves them out
    e.r['EAX'] = e.r['ECX']


B = Bench([(0x408f00, 0x40914f), (0x421770, 0x4217e0), (0x421690, 0x421720), (0x590730, 0x590750), (0x424cf0, 0x424d10), (0x67df40, 0x67e040),
           (0x67faf0, 0x67fb20), (0x682720, 0x682780), (0x5f5fc0, 0x5f7f00), (0x67a400, 0x690000),
           (0x407b80, 0x407b8a), (0x407b90, 0x407c2e), (0x407c90, 0x407d79), (0x421510, 0x421588), (0x5da3c0, 0x5da3c1),
           (0x409620, 0x40978d), (0x424cc0, 0x424ce2), (0x5e2a30, 0x5e2a7e),
           (0x409260, 0x409296), (0x402ef0, 0x402f10), (0x4030c0, 0x4030c4),
           (0x4099b0, 0x409b0b), (0x424d10, 0x424d2e), (0x424e10, 0x424e6e), (0x485ac0, 0x485af4), (0x6ace60, 0x6acea0), (0x6adc80, 0x6add25), (0x5f5660, 0x5f5690),
           (0x4088a0, 0x408ef8), (0x4081e0, 0x408464), (0x41d7c0, 0x41e250), (0x421590, 0x421630), (0x421680, 0x421690)] + fileopen.RANGES,
          hooks={0x691de0: lock, 0x691ea0: lambda e: None})
NAMES = [0x786b68, 0x785e78, 0x785f98, 0x7862c8, 0x785f68, 0x7862f8, 0x786d10, 0x7866e8, 0x7862e0, 0x785ed8, 0x785f80, 0x786aa8, 0x786040, 0x7867d8]
WORDS = [b'Font', b'Sky', b'Rides', b'Shops', b'Staff', b'Water', b'Lake', b'Paths', b'Fence', b'Trees', b'Rock', b'Hint', b'Map', b'Sound']
REF, LST, NODES, OBJS, TEXTS, VT = RAM, RAM + 0x100, RAM + 0x200, RAM + 0x1000, RAM + 0x2000, RAM + 0x3000


def varied(r, base):
    return bytes(c ^ 0x20 if r.random() < 0.3 else c for c in base) + bytes(0x10)


def remove(r):
    regions = {HEAP: heap(), LOG: log_region(), VT: vtable({0: 1}), 0x786b9c: struct.pack('<I', REF)}
    for k, (at, word) in enumerate(zip(NAMES, WORDS)):
        key = bytearray(r.bytes(0x14))
        put(key, 8, 'I', TEXTS + 0x20 * k)
        regions[at] = bytes(key)
        regions[TEXTS + 0x20 * k] = word + bytes(0x10)
    n = r.choice([0, 1, 2, 3, 4, 6])
    lst = bytearray(r.bytes(0x20))
    put(lst, 4, 'III', NODES if n else 0, NODES + 0x10 * (n - 1) if n else 0, n)
    nodes = bytearray(r.bytes(0x10 * 6))
    for k in range(n):
        held = OBJS + 0x40 * k
        put(nodes, 0x10 * k, 'III', held, NODES + 0x10 * (k + 1) if k + 1 < n else 0, NODES + 0x10 * (k - 1) if k else 0)
        o = bytearray(struct.pack('<I', VT) + r.bytes(0x3c))
        put(o, 0xc, 'I', TEXTS + 0x400 + 0x20 * k)
        regions[held] = bytes(o)
        regions[TEXTS + 0x400 + 0x20 * k] = varied(r, r.choice(WORDS) if r.random() < 0.75 else r.choice([b'Fonts', b'Sk', b'', b'Other']))
    ref = bytearray(r.bytes(8))
    put(ref, 0, 'I', LST if r.random() < 0.92 else 0)
    regions.update({REF: bytes(ref), LST: bytes(lst), NODES: bytes(nodes)})
    return Call(0x408f00, [], regions, ret='a')


OWNER, WSTR = RAM + 0x4000, 0x7861c0


def ctor(r):
    n = r.choice([0, 1, 5, 0x10, 0x23])
    wide = bytearray(r.bytes(0x14))
    put(wide, 0, 'III', 0x7030c8, n, TEXTS)
    regions = {OWNER: r.bytes(0x40), WSTR: bytes(wide), TEXTS: r.bytes(0x60), HEAP: heap(fail=r.choice([0, 0, 0, 0, 0, 1, 2, 3])), **global_heap()}
    return Call(0x407b90, [OWNER], regions, conv='t', ret='a')


def chain(r, regions, at, count, objects, vt):
    """count nodes {object, next, prev} at `at` (16 bytes apart), each object (or none) with vtable vt; returns the head and tail."""
    nodes = bytearray(r.bytes(0x10 * max(count, 1)))
    for k in range(count):
        held = objects + 0x40 * k if r.random() < 0.85 else 0
        put(nodes, 0x10 * k, 'III', held, at + 0x10 * (k + 1) if k + 1 < count else 0, at + 0x10 * (k - 1) if k else r.getrandbits(32))
        if held:
            regions[held] = put(bytearray(r.bytes(0x40)), 0, 'I', vt)
    regions[at] = bytes(nodes)
    return (at, at + 0x10 * (count - 1)) if count else (0, 0)


def dtor(r, address):
    owner = 0x786b68 if address == 0x407b80 else OWNER
    regions = {HEAP: heap(), LOG: log_region(), VT: vtable({0: 1, 3: 1}), **global_heap()}
    o = bytearray(r.bytes(0x40))
    put(o, 0, 'III', 0x703090, r.randrange(8), TEXTS + 0x100)
    n = r.choice([0, 1, 2, 3, 5])
    head, tail = chain(r, regions, NODES, n, OBJS, VT)
    put(o, 0x28, 'III', head, tail, r.choice([n, n, n, r.randrange(n + 1)]))
    if r.random() < 0.8:  # the registry: {list, cached, ?}; the list {vtable, head, tail, count}
        m = r.choice([0, 1, 3])
        lst = bytearray(r.bytes(0x20))
        put(lst, 0, 'IIII', VT, *chain(r, regions, NODES + 0x100, m, OBJS + 0x400, VT), m)
        ref = bytearray(r.bytes(0xc))
        put(ref, 0, 'I', LST if r.random() < 0.85 else 0)
        regions.update({REF: bytes(ref), LST: bytes(lst)})
        put(o, 0x34, 'I', REF)
    else:
        put(o, 0x34, 'I', 0)
    put(o, 0x38, 'I', r.choice([0, 0, TEXTS + 0x200]))
    regions[owner] = bytes(o)
    if address == 0x407b80:
        return Call(address, [], regions, ret='v')
    return Call(address, [owner], regions, conv='t', ret='v')


def string(regions, at, text):
    regions[at] = put(bytearray(0x14), 0, 'IIIII', 0x703090, len(text), at + 0x20, len(text) + 1, 8) + bytes(0xc) + text + bytes(0x10)


def add(r):
    regions = {HEAP: heap(fail=r.choice([0] * 12 + [1, 2])), LOG: log_region(), **global_heap()}
    o = bytearray(r.bytes(0x40))
    n = r.choice([0, 1, 2, 3, 5, 5])
    words = [varied(r, r.choice(WORDS + [b'Fonts', b'Sk', b''])).rstrip(bytes(1)) for _ in range(n)]
    if r.random() < 0.6:
        words.sort(key=bytes.lower)
    nodes = bytearray(r.bytes(0x10 * max(n, 1)))
    for k, word in enumerate(words):
        held = OBJS + 0x40 * k
        string(regions, held, word)
        held = HEAP + 0x10 if r.random() < 0.05 else held   # (the block the new String gets)
        put(nodes, 0x10 * k, 'III', held, NODES + 0x10 * (k + 1) if k + 1 < n else 0, NODES + 0x10 * (k - 1) if k else r.getrandbits(32))
    regions[NODES] = bytes(nodes)
    count = r.choice([n, n, n, n, 0, 1, 7]) if n else r.choice([0] * 20 + [1])
    put(o, 0x24, 'IIII', 0x6fd704, NODES if n else 0, NODES + 0x10 * (n - 1) if n else 0, count)
    regions[OWNER] = bytes(o)
    string(regions, RAM + 0x4800, varied(r, r.choice(WORDS + [b'Fonts', b'Sk', b'', b'Zoo', b'Aa'])).rstrip(bytes(1)))
    return Call(0x409620, [OWNER, RAM + 0x4800], regions, conv='t', ret='v')


THEMES = [(0x786478, b'fantasy'), (0x786808, b'halloween'), (0x786970, b'jungle'), (0x7869e8, b'space')]
LABEL, NAME, THEMETEXT, RECS, CONV = RAM + 0x6a00, RAM + 0x6b00, RAM + 0x6c00, RAM + 0x6e00, RAM + 0x6f00


def wide(s):
    return b''.join(bytes([c, 0]) for c in s) + bytes(2)


def theme(r, op):
    """0x409a30 / 0x4099b0: a theme name (one of the four, case varied, cut short or not, or another) shown on a label (a recorder's slot 5) as
    the text of the record table [0x78bad4] (0-5 records, some not terminated) widened by the converter [0x78ba7c] (on or off, as language.py)."""
    log = log_region()
    lang = bytearray(r.bytes(0x600))   # (0x78b530: the converter +0x54c, its switch +0x550, the table +0x5a4)
    conv = bytearray(r.bytes(0x10))
    put(conv, 8, 'BBHI', r.choice([0x80, 0x81, 0, 0xff]), r.getrandbits(8), r.choice([0, 3, 0x80, 0x400, r.randrange(0x800)]), RAM + 0x8000)
    put(lang, 0x54c, 'II', CONV, r.choice([0, 1, 1, 1]))
    put(lang, 0x5a4, 'I', RECS)
    count = r.choice([0, 1, 2, 3, 4, 5, 5, 5])
    recs = bytearray(r.bytes(0x100))
    put(recs, 8, 'II', count, RECS + 0x40)
    pool = bytearray()
    for i in range(5):
        body = bytes(r.randrange(1, 0x100) for _ in range(r.choice([0, 1, 3, 6, 9]))) + bytes([0]) * r.choice([1, 1, 2])
        rec = bytearray(struct.pack('<I', r.choice([1] * 6 + [2, 0x101]) | len(body) << 8) + body + bytes(0x3c - len(body)))
        if r.random() < 0.15:
            rec[4 + ((len(body) + 3) & ~3)] = 1   # (not terminated)
        pool += rec
    struct.pack_into('<5I', recs, 0x40, *[RAM + 0x7000 + 0x40 * i for i in range(5)])
    regions = {LOG: log, 0x78b530: lang, CONV: conv, RAM + 0x8000: r.bytes(0x1000), RECS: recs, RAM + 0x7000: bytes(pool),
               0x7ba058: bytes([0x55]) * 0x404, HEAP: heap(fail=r.choice([0] * 12 + [1, 2, 3])), LABEL:struct.pack('<I', LABEL + 0x40) + bytes(0x3c) + vtable({5: 1})}
    texts = bytearray(0x100)
    for i, (at, word) in enumerate(THEMES):
        regions[at] = struct.pack('<5I', 0x7030c8, len(word), THEMETEXT + 0x40 * i, len(word) + 1, 8)
        texts[0x40 * i:0x40 * i + 2 * len(word) + 2] = wide(word)
    regions[THEMETEXT] = bytes(texts)
    word = bytes(c ^ 0x20 if r.random() < 0.3 else c for c in r.choice([w for _, w in THEMES] * 3 + [b'', b'spac', b'jungles', b'zoo']))
    if r.random() < 0.1:
        word = word[:r.randrange(len(word) + 1)]
    if op == 'Theme':
        regions[NAME] = struct.pack('<5I', 0x7030c8, len(word), NAME + 0x40, 0, 8) + bytes(0x2c) + wide(word) + bytes(0x10)
        return Call(0x409a30, [0x786b68, NAME, LABEL], regions, conv='t', ret='v')
    table = import_table(log, {'MultiByteToWideChar': (40, 0x6fd174, 6, 4, 0)})
    for at, stub in global_heap().items():
        table[at - IMPORT_TABLE:at - IMPORT_TABLE + 4] = stub
    regions[IMPORT_TABLE] = table
    text = wide(word)
    units = r.choice([len(text) // 2] * 6 + [0, 0, 1, max(0, len(text) // 2 - 2)])
    set_return(log, 40, units)
    fill(log, 40, 4, text)
    regions[NAME] = struct.pack('<5I', 0x703090, len(word), NAME + 0x40, 0, 8) + bytes(0x2c) + word + bytes(0x10)
    return Call(0x4099b0, [0x786b68, NAME, LABEL], regions, conv='t', ret='v')


APPENDS = [0x7869d0, 0x7864f0, 0x7865f8, 0x786640, 0x786388, 0x785f38, 0x7868e0, 0x786778, 0x786af0, 0x7863b8, 0x786850, 0x7869a0, 0x7869b8]
ROOT, SEEN, LISTED = RAM + 0x5000, RAM + 0x5100, RAM + 0x5200
PARTS = [b'Fonts', b'data\\sky', b'Rides', b'x', b'', b'Lake\\']


def mounts(r):
    """The regions both folder ops share: the file imports as fileopen.py's, mostly failing (CreateFileA / GetFileAttributesA /
    FindFirstFileA), now and then finding a folder or a WAD; the heap now and then failing an allocation (a failed object block:
    AddFolderAndWad on a null object faults in both); ".wad", "\\" and the owner, whose registry ({list, cached}) has a list of 0-2
    nodes or none (made by the first add)."""
    log = log_region()
    for imp, k in fileopen.K.items():
        set_return(log, k, r.choice({'CreateFileA': [0xffffffff] * 3 + [0x77], 'GetFileAttributesA': [0xffffffff] * 3 + [0x10, 0x20],
                                     'FindFirstFileA': [0xffffffff] * 3 + [0x99], 'ReadFile': [1, 0], 'GetFileSize': [0, 0x58],
                                     'InterlockedDecrement': [0, 1], 'InterlockedIncrement': [1, 2], 'CreateSemaphoreA': [0, 0x44],
                                     'mmioOpenA': [0, 0x66], 'IsDBCSLeadByte': [0], 'GetCommandLineA': [0]}.get(imp, [0, 1])))
    fill(log, fileopen.K['ReadFile'], 1, bytes(fileopen.wad_header(r)), 0)
    globals_ = bytearray(r.bytes(0x318))
    globals_[0:8] = b'tp.exe\0\0'
    globals_[0x108:0x10f] = b'C:\\tp\\\0'
    regions = {HEAP: heap(0x40000, fail=r.randrange(1, 16) if r.random() < 0.08 else 0), LOG: log, IMPORT_TABLE: import_table(log, fileopen.IMPORTS),
               fileopen.GLOBALS: globals_, fileopen.PATHS: r.bytes(0x210), 0xfb1440: bytes(4)}
    string(regions, fileopen.WAD_EXT, b'.wad')
    string(regions, fileopen.SLASH, b'\\' if r.random() < 0.95 else b'/')
    n = r.choice([0, 0, 1, 2])
    lst = bytearray(r.bytes(0x10))
    put(lst, 0, 'IIII', 0x6fdb0c, NODES if n else 0, NODES + 0x10 * (n - 1) if n else 0, n)
    nodes = bytearray(r.bytes(0x20))
    for k in range(n):
        put(nodes, 0x10 * k, 'III', SEEN + 0x40 * k, NODES + 0x10 * (k + 1) if k + 1 < n else 0, NODES + 0x10 * (k - 1) if k else 0)
    ref = put(bytearray(r.bytes(0xc)), 0, 'I', LST if r.random() < 0.8 else 0)
    o = put(bytearray(r.bytes(0x40)), 0x34, 'I', REF)
    regions.update({REF: bytes(ref), LST: bytes(lst), NODES: bytes(nodes), SEEN: r.bytes(0x80), OWNER: bytes(o)})
    return regions


def wstring(regions, at, text):
    regions[at] = struct.pack('<5I', 0x7030c8, len(text), at + 0x20, len(text) + 8, 8) + bytes(0xc) + wide(text) + bytes(8)


def folders(r):
    """0x4088a0 (mounts()): a root (a WString: a path ending in "\\" or not, or empty) and the globals' names and folder parts (Strings,
    now and then empty)."""
    regions = mounts(r)
    for k, (at, word) in enumerate(zip(NAMES, WORDS)):
        string(regions, at, varied(r, word).rstrip(bytes(1)) if r.random() < 0.95 else b'')
    for at in APPENDS:
        string(regions, at, r.choice(PARTS))
    wstring(regions, ROOT, r.choice([b'C:\\tp\\data', b'C:\\tp\\data\\', b'data', b'', b'\\']))
    return Call(0x4088a0, [OWNER, ROOT], regions, conv='t', ret='v')


def listed(r):
    """0x4081e0 (mounts()): a list at [0x786b90] of 0-3 names (Strings), the root at 0x785e48 (a WString, assigned to a String: its
    bytes, sic), the three folder parts and names (Strings) and the second root 0xf816d4 (a String) for the third object's add."""
    regions = mounts(r)
    n = r.choice([0, 1, 1, 2, 3])
    nodes = bytearray(r.bytes(0x20))
    for k in range(n):
        put(nodes, 8 * k, 'II', LISTED + 0x40 + 0x40 * k, LISTED + 8 * (k + 1) if k + 1 < n else 0)
        string(regions, LISTED + 0x40 + 0x40 * k, r.choice([b'users', b'online', b'', b'a\\b']))
    regions[LISTED] = bytes(nodes)
    regions[0x786b90] = struct.pack('<I', LISTED if n else 0)
    wstring(regions, 0x785e48, r.choice([b'C:\\tp', b'save\\', b'']))
    for at in (0x786430, 0x785e90, 0x786670, 0x786a48, 0x786ce0, 0x7860e8):
        string(regions, at, r.choice(PARTS))
    string(regions, fileopen.ROOT2, r.choice([b'', b'C:\\tp', b'C:\\tp\\', b'data']))
    return Call(0x4081e0, [OWNER], regions, conv='t', ret='v')


def case(r):
    op = r.choice(OPS)
    if op == 'Folders':
        return folders(r)
    if op == 'Listed':
        return listed(r)
    if op in ('Theme', 'ThemeAnsi'):
        return theme(r, op)
    if op == 'Ctor':
        return ctor(r)
    if op == 'Dtor':
        return dtor(r, 0x407c90)
    if op == 'Static':
        return dtor(r, 0x407b80)
    if op == 'Add':
        return add(r)
    if op == 'Flags':   # 0x409260: an owner's +0x18 set or not, the flags, 0x7477e4 and a game clock of any bytes
        owner = put(bytearray(r.bytes(0x40)), 0x18, 'I', r.choice([0, 0, 1, 5]))
        regions = {RAM: owner, 0x7a1a10: r.bytes(8), 0x7477e0: r.bytes(8), 0x785970: r.bytes(0x60)}
        return Call(0x409260, [RAM], regions, conv='t', ret='a')
    return remove(r)


OPS = [k for k in os.environ.get('OPS', 'Remove,Ctor,Dtor,Static,Add,Flags,Theme,ThemeAnsi,Folders,Listed').split(',') if k]

if __name__ == '__main__':
    B.main(case)
