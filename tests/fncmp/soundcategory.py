# A sound category's SFX.map loading (src/game/core/soundcategory) against the originals: the image fixups (0x6c0510 and below: a valid image of
# 1 or 2 categories, 0-3 sounds each, 0-3 variations, 0-3 samples and 0-3 links, built in the order the fixup walks it; the samples' banks 0 or
# within deque C, the weights random, delta-coded or not) over a category object whose deques A (the category records, each with a PtrList of
# 0-3 banks) and C (the bank indices) are small stretches of one or two blocks (the block boundary crossed now and then); a variation's sample
# count (0x6bd7a0) and the simple sound mark (0x6c0550: each of its conditions met or missed). The free slots in deques A and B (0x6bf150 with
# holes in A, 0x6bf1c0 over bank objects and gaps, long runs crossing into the second block); the BANK.map entries read (0x6bf9e0) through a stream recorder whose reads hand out the file's bytes in turn (seq_read).
# usage: soundcategory.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,...)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import fileopen  # noqa: E402  (the file library's ranges, imports and globals)
from fncmp import HEAP, IMPORT_TABLE, LOG, RAM, Bench, Call, echo, fill, fill_arg, heap, import_table, log_region, recorder, seq_read, set_return, strings, vtable  # noqa: E402

CALLS = {'count': 0x6bd7a0, 'simple': 0x6c0550, 'variation': 0x6c06e0, 'sound': 0x6c0610, 'record': 0x6c05b0, 'fixup': 0x6c0510, 'readimage': 0x6c0360,
         'readmap': 0x6c0450, 'freecat': 0x6bf150, 'freebank': 0x6bf1c0, 'entries': 0x6bf9e0, 'bankdtor': 0x6c01b0, 'bankdel': 0x6c0190,
         'openfolder': 0x6bf850, 'setfolder': 0x6bf260, 'findbank': 0x6bfa60, 'readbankmap': 0x6bfe40, 'loadbankmap': 0x6c0210,
         'addref': 0x6338c0, 'release': 0x6338d0}
B = Bench([(0x6bd7a0, 0x6bd7ac), (0x6bf150, 0x6bf31c), (0x6bf850, 0x6c0303), (0x6c0310, 0x6c035f), (0x6c0360, 0x6c0844), (0x4036a0, 0x4036e0), (0x639100, 0x639120),
           (0x6b9b30, 0x6b9b94), (0x5e42d0, 0x5e431c), (0x6338b0, 0x633900), (0x6c1940, 0x6c1b00), (0x6c2990, 0x6c29c0), (0x4f6740, 0x4f67c0), (0x632200, 0x632230)]
          + fileopen.RANGES)
CAT, OUT, RECS, NODES = RAM, RAM + 0x100, RAM + 0x200, RAM + 0x400
MAPA, MAPC, MAPB = RAM + 0x600, RAM + 0x640, RAM + 0x680
BLOCKA, BLOCKC, BLOCKB = RAM + 0x10000, RAM + 0x20000, RAM + 0x40000   # (two blocks each, 0x1000 apart, only the used stretch mapped)
IMAGE = RAM + 0x3000
STREAM, SRC, VT_STREAM, VT_SRC, VT_HANDLE, FILE = RAM + 0x700, RAM + 0x740, RAM + 0x780, RAM + 0x800, RAM + 0x880, RAM + 0x5000
SFX_GUID, BANK_GUID = bytes.fromhex('002c61e9d031d211b40900b0c993f203'), bytes.fromhex('012c61e9d031d211b40900a0c993f203')
STREAM_VTABLE, HANDLE_VTABLE, REFOBJECT2_VTABLE, STRING_VTABLE, BANK_VTABLE = 0x703100, 0x703180, 0x703898, 0x703090, 0x70a960
BANKS, NAMES, OBJS, PATHTEXT = RAM + 0x6000, RAM + 0x7000, RAM + 0x8000, RAM + 0x9000   # bank objects, their names, their handles' objects, those paths
VT_OBJ, FOLDER, FOLDERTEXT, FOUND = RAM + 0x900, RAM + 0x980, RAM + 0x9c0, RAM + 0xa40
SOUND, SOUNDTEXT, NEGATIVE, PATHSTR = RAM + 0xb00, RAM + 0xb80, RAM + 0xbc0, RAM + 0xc00   # the sound system (*0xfb1ffc, a String at +0x30), -1
SOUND_SYSTEM = 0xfb1ffc
FOLDERS = [b'C:\\tp\\data\\', b'data', b'data\\snd', b'', b'\\', b'a\\b.wad\\x', b'C:\\tp\\data\\x.wad', b'Global\\']
NAMES_GIVEN = [b'Bank0.bnk', b'x', b'', b'sub\\b.bnk', b'Bank1.bnk']
# The imports' answers (as fileopen.py's): the folder and WAD opens below SoundCategory_OpenBankFolder run the file library through them.
ANSWERS = {'CreateFileA': [0xffffffff, 0x77, 0x77, 0x77], 'GetFileAttributesA': [0xffffffff, 0x10, 0x20, 0x20, 0x11, 0x80], 'ReadFile': [1, 1, 0],
           'IsDBCSLeadByte': [0, 0, 1], 'mmioOpenA': [0, 0x66, 0x66], 'FindFirstFileA': [0xffffffff, 0x99], 'InterlockedDecrement': [0, 1, 1, 2],
           'InterlockedIncrement': [1, 2], 'CreateSemaphoreA': [0, 0x44, 0x44], 'WaitForSingleObject': [0, 0, 0x102], 'GetCommandLineA': [0],
           'GetFileSize': [0, 0x58, 0x1000]}
REFC, REFC_VT = RAM + 0xd00, RAM + 0xd80   # a counted object (the base class 0x703770's) and its recorder vtable
OPS = [n for n in CALLS if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]


def deque(r, at, mapat, values, regions):
    """A deque's begin iterator (16 bytes) over values starting at a random place in its first block at `at` (into the next, at + 0x1000 or + 0x3000,
    when they run past it); maps the stretch of each block they use and the map's two slots."""
    n = len(values)
    start = max(0, min(0x3ff, r.choice([0, 0x3ff - n // 2, 0x400 - n, r.randrange(max(1, 0x400 - n + 1))])))
    blocks = [at, at + r.choice([0x1000, 0x3000])]
    regions[mapat] = struct.pack('<2I', *blocks)
    cells = {}
    for i, v in enumerate(values):
        q = start + i
        cells[blocks[q // 0x400] + 4 * (q % 0x400)] = v
    for b in blocks:
        mine = sorted(a for a in cells if b <= a < b + 0x1000)
        if mine:
            buf = bytearray(r.bytes(mine[-1] + 4 - mine[0]))
            for a in mine:
                struct.pack_into('<I', buf, a - mine[0], cells[a])
            regions[mine[0]] = buf
    return struct.pack('<4I', at, at + 0x1000, at + 4 * start, mapat)


def deque_full(r, at, mapat, values, regions):
    """deque() and the rest of the deque after its begin iterator: the end iterator (at the end of its block when the values fill it, as the
    deque's push_back leaves it), the map (2 slots) and the count (0x2c bytes, for the deque + 4)."""
    it = deque(r, at, mapat, values, regions)
    first, _last, cur, _node = struct.unpack('<4I', it)
    blocks = struct.unpack('<2I', regions[mapat])
    q = (cur - first) // 4 + len(values)
    b = q // 0x400 - 1 if q and q % 0x400 == 0 else q // 0x400
    return it + struct.pack('<4I', blocks[b], blocks[b] + 0x1000, blocks[b] + 4 * (q - 0x400 * b), mapat + 4 * b) + struct.pack('<3I', mapat, 2, len(values))


def files(r, log, regions, wad=True):
    """The file library's world (fileopen.py's): the imports recorders and their answers, a WAD header for ReadFile, the globals, the heap. Without
    wad (the log's two fills taken), GetFileAttributesA always answers none: no WAD is found and no folder opens."""
    for imp, k in fileopen.K.items():
        if r.random() < 0.9:
            set_return(log, k, r.choice(ANSWERS.get(imp, [0, 1, r.getrandbits(32)])))
    if r.random() < 0.8:
        echo(log, fileopen.K['SetFilePointer'], 1)
    if wad:
        fill(log, fileopen.K['ReadFile'], 1, bytes(fileopen.wad_header(r)), 0)
        fill_arg(log, fileopen.K['ReadFile'], 3, 2, 1)
    else:
        set_return(log, fileopen.K['GetFileAttributesA'], 0xffffffff)
    strings(log, 4, 0, out=7)   # (a recorder object's +0x10, Vfn4's target: stack addresses)
    set_return(log, fileopen.K['InterlockedDecrement'], r.choice([1, 1, 2]))   # (a count to 0 tears the game's abstract RefObject2 down: _purecall)
    base = r.choice([b'C:\\tp\\', b'D:\\'])
    globals_ = bytearray(r.bytes(0x318))
    globals_[0:8] = b'tp.exe\0\0'
    globals_[0x108:0x109 + len(base)] = base + b'\0'
    regions.update({IMPORT_TABLE: import_table(log, fileopen.IMPORTS), fileopen.GLOBALS: globals_, fileopen.PATHS: r.bytes(0x210),
                    fileopen.LOCALE: bytes(4), VT_OBJ: vtable(fileopen.OBJECT_SLOTS), HEAP: heap(0x10000)})


def string(r, text, at):
    """A String over text at `at` (its capacity the length's step of 8, or more)."""
    return struct.pack('<5I', STRING_VTABLE, len(text), at, (len(text) & ~7) + r.choice([8, 8, 16]), 8)


def handle_object(r, i, path, regions, recorders=True):
    """The RefObject2 a bank's (or the category's) Handle holds: the game's base class (its Vfn4 gives the path; with recorders, now and then a
    recorder object), with a count of 1-3."""
    at = OBJS + 0x40 * i
    obj = bytearray(r.bytes(0x40))
    obj[0:0x24] = struct.pack('<4I', REFOBJECT2_VTABLE if not recorders or r.random() < 0.85 else VT_OBJ, 0, r.choice([0, 0x44]), r.randrange(1, 4)) + string(r, path, PATHTEXT + 0x40 * i)
    regions[at] = obj
    regions[PATHTEXT + 0x40 * i] = path + b'\0' + r.bytes(8)
    return at


def bank(r, i, name, path, regions, recorders=True):
    """Bank i (0x2c bytes): its vtable, count, name and folder Handle (its object's path given, or no object)."""
    at = BANKS + 0x40 * i
    b = bytearray(r.bytes(0x40))
    obj = handle_object(r, i, path, regions, recorders) if r.random() < 0.85 else 0
    b[0:0x2c] = struct.pack('<3I', BANK_VTABLE, r.randrange(1, 3), r.getrandbits(32)) + string(r, name, NAMES + 0x40 * i) + struct.pack('<3I', r.getrandbits(8), HANDLE_VTABLE, obj)
    regions[at] = b
    regions[NAMES + 0x40 * i] = name + b'\0' + r.bytes(8)
    return at


def category(r, banks=4, holes=False, bank_objects=None):
    """The category object with deques A (records with bank lists) and C (bank indices); returns regions."""
    regions = {HEAP: heap(0x2000)}
    nrec = r.randrange(1, 4)
    recs = bytearray(r.bytes(0x40 * nrec))
    nodes = bytearray(r.bytes(0x10 * 9))
    for i in range(nrec):
        n = r.choice([0, 0, 1, 2, 3])
        vals = r.sample(range(1, 7), n)
        addr = [NODES + 0x10 * (3 * i + j) for j in range(n)]
        for j in range(n):
            struct.pack_into('<3I', nodes, addr[j] - NODES, vals[j], addr[j + 1] if j + 1 < n else 0, addr[j - 1] if j else 0)
        struct.pack_into('<4I', recs, 0x40 * i + 0x24, r.getrandbits(32), addr[0] if n else 0, addr[-1] if n else 0, n)
    regions[RECS] = recs
    regions[NODES] = nodes
    cat = bytearray(r.bytes(0xd0))
    cat[0x3c:0x68] = deque_full(r, BLOCKA, MAPA, [0 if holes and r.random() < 0.3 else RECS + 0x40 * i for i in range(nrec)], regions)
    cat[0x9c:0xc8] = deque_full(r, BLOCKC, MAPC, [r.choice([r.randrange(1, 7), r.getrandbits(32)]) for _ in range(banks)], regions)
    if bank_objects is not None:
        cat[0x6c:0x98] = deque_full(r, BLOCKB, MAPB, bank_objects, regions)
    struct.pack_into('<I', cat, 0xc8, r.randrange(nrec))
    cat[0xcc] = r.choice([0, 1, 1, 0x80])
    regions[CAT] = cat
    return regions


def image(r, banks, levels, top):
    """An SFX.map image from its records down, in the fixup's order; levels: which record kinds come first ('c' categories, 's' sounds, 'v' variations).
    Returns (bytes, the count of top records)."""
    def link(nvars):
        return struct.pack('<I', r.choice([r.randrange(1, nvars + 1), r.randrange(1, nvars + 1), r.getrandbits(32)])) + r.bytes(4)

    def sample():
        s = bytearray(r.bytes(0x10))
        struct.pack_into('<H', s, 0xc, r.choice([0, r.randrange(1, banks + 1), r.randrange(1, banks + 1)]))
        return bytes(s)

    def variation_head(nsamp, nlinks):
        v = bytearray(r.bytes(0x2a))
        struct.pack_into('<HHI', v, 0, nsamp, r.getrandbits(16), nlinks)
        if r.random() < 0.3:
            struct.pack_into('<2H', v, 0x10, 0, 0)
        return v

    def variations(n):   # the heads, then each one's samples and links
        heads, tails = [], []
        for _ in range(n):
            ns, nl = r.choice([0, 1, 1, 2, 3]), r.choice([0, 0, 1, 2, 3])
            heads.append(variation_head(ns, nl))
            tails.append(b''.join(sample() for _ in range(ns)) + b''.join(link(n) for _ in range(nl)))
        return b''.join(heads) + b''.join(tails)

    def sounds(n):
        heads, tails = [], []
        for _ in range(n):
            nv = r.choice([0, 1, 1, 2, 3])
            s = bytearray(r.bytes(0x14))
            struct.pack_into('<II', s, 0, r.getrandbits(16), nv)
            struct.pack_into('<H', s, 0x10, r.choice([6, 0xe, 2, 4, r.getrandbits(16)]))
            heads.append(s)
            tails.append(variations(nv))
        return b''.join(heads) + b''.join(tails)

    def categories(n):
        heads, tails = [], []
        for _ in range(n):
            ns = r.choice([0, 1, 2, 3])
            c = bytearray(r.bytes(0x18))
            struct.pack_into('<I', c, 0, ns)
            heads.append(c)
            tails.append(sounds(ns))
        return b''.join(heads) + b''.join(tails)

    body = {'c': categories, 's': sounds, 'v': variations}[levels](top)
    return bytearray(body) + bytearray(r.bytes(0x10)), top


def case(r):
    name = r.deal(OPS)
    if name in ('addref', 'release'):   # the Interlocked imports' answers (0 now and then: the object deleted through its vtable +8)
        regions = {LOG: log_region()}
        files(r, regions[LOG], regions)
        regions[REFC_VT] = vtable({2: 1}, 4)
        regions[REFC] = struct.pack('<2I', REFC_VT, r.getrandbits(32)) + r.bytes(8)
        k = fileopen.K['InterlockedIncrement' if name == 'addref' else 'InterlockedDecrement']
        set_return(regions[LOG], k, r.choice([0, 0, 1, 2, 0xffffffff]))
        return Call(CALLS[name], [REFC], regions, conv='t', ret='a')
    if name == 'count':
        regions = {IMAGE: r.bytes(8)}
        return Call(CALLS[name], [IMAGE], regions, conv='c', ret='a')
    if name == 'simple':   # a sound and its variation, each condition met mostly
        s = bytearray(r.bytes(0x14))
        v = bytearray(r.bytes(0x2a))
        struct.pack_into('<I', s, 4, 1 if r.random() < 0.8 else r.choice([0, 2, r.getrandbits(32)]))
        struct.pack_into('<I', s, 8, IMAGE + 0x40)
        struct.pack_into('<H', s, 0x10, r.choice([6, 6, 0xe, 0xfffe, 2, 4, r.getrandbits(16)]))
        struct.pack_into('<I', v, 0, (r.getrandbits(16) << 16) | (1 if r.random() < 0.8 else r.choice([0, 2])))
        struct.pack_into('<I', v, 0x26, 0 if r.random() < 0.8 else r.getrandbits(32))
        struct.pack_into('<2H', v, 0x10, *(0, 0) if r.random() < 0.7 else r.choice([(1, 0), (0, 1), (r.getrandbits(16), r.getrandbits(16))]))
        regions = {CAT: r.bytes(0xd0), IMAGE: s, IMAGE + 0x40: v}
        return Call(CALLS[name], [CAT, IMAGE], regions, conv='t', ret='v')
    banks = r.randrange(1, 7)
    regions = category(r, banks)
    if name in ('readimage', 'readmap'):   # SFX.map's bytes, read in turn (a sequenced read)
        top = r.choice([0, 1, 1, 2])
        body, top = image(r, banks, 'c', top)
        guid = r.choice([SFX_GUID, SFX_GUID, SFX_GUID, BANK_GUID, bytes(16)])
        data = guid + struct.pack('<4I', r.choice([0, 0, 1]), 0, top, 0)[:12] + bytes(body)
        regions[FILE] = data
        log = log_region()
        size, pos = r.choice([(len(data), 0x14), (len(data), 0x14), (len(data), 0x14), (len(data) - 8, 0x14), (len(data) + 0x20, 0x14), (0x1c, 0x14)])
        regions[LOG] = log
        regions[HEAP] = heap(0x3000)
        if name == 'readimage':
            seq_read(log, 2, FILE, len(data), buffer=0, count=1)
            strings(log, 2, 0, out=1)
            set_return(log, 5, size)
            set_return(log, 6, pos)
            regions[STREAM] = struct.pack('<2I', VT_STREAM, r.getrandbits(32))
            regions[VT_STREAM] = vtable({2: 2})
            regions[OUT] = r.bytes(8)
            return Call(CALLS[name], [CAT, OUT, STREAM, OUT + 4], regions, conv='t', ret='a')
        # the category's Handle (+0x30) opens the stream: its slot 3 answers &status, which it fills, and gives the stream SRC (a recorder source)
        status = r.choice([0, 0, 0, 1, -1, -0x80000000])
        struct.pack_into('<2I', regions[CAT], 0x30, VT_HANDLE, r.getrandbits(32))
        regions[VT_HANDLE] = vtable({3: 5})
        echo(log, 3, 0)
        fill(log, 3, 0, struct.pack('<i', status), 0)
        fill(log, 3, 1, struct.pack('<2I', STREAM_VTABLE, SRC), 1)
        strings(log, 3, 4, out=3)
        seq_read(log, 1, FILE, len(data), buffer=0, count=1)
        strings(log, 1, 0, out=1)
        set_return(log, 4, size)
        set_return(log, 5, pos)
        regions[VT_SRC] = vtable({0: 1, 1: 2})
        regions[SRC] = struct.pack('<3I', VT_SRC, 0, r.choice([1, 2]))
        regions[IMPORT_TABLE] = import_table(log, {'InterlockedDecrement': (51, 0x6fd12c, 1, 0, 0)})
        set_return(log, 51, r.choice([0, 1]))
        regions[OUT] = r.bytes(8)
        regions[STREAM] = b'cat_x' + r.choice([b'SFX.map', b'']) + bytes(1)
        return Call(CALLS[name], [CAT, OUT, STREAM, OUT + 4], regions, conv='t', ret='a')
    if name in ('bankdtor', 'bankdel'):
        regions = {LOG: log_region()}
        files(r, regions[LOG], regions)
        at = bank(r, 0, r.choice(NAMES_GIVEN), r.choice(FOLDERS), regions)
        if name == 'bankdtor':
            return Call(CALLS[name], [at], regions, conv='t', ret='v')
        return Call(CALLS[name], [at, r.choice([0, 1, 1, 3, 2])], regions, conv='t', ret='a')
    if name in ('openfolder', 'setfolder'):   # a folder (a String, or chars) opened as a WAD (its .wad there) or a folder through the file library
        regions = category(r, banks)
        regions[LOG] = log_region()
        files(r, regions[LOG], regions)
        text = r.choice(FOLDERS)
        regions[FOLDERTEXT] = text + b'\0' + r.bytes(8)
        if name == 'openfolder':
            regions[FOLDER] = string(r, text, FOLDERTEXT)
            obj = handle_object(r, 0, r.choice(FOLDERS), regions) if r.random() < 0.3 else 0
            regions[FOUND] = struct.pack('<B3x2I', r.choice([0, 0, 0, 1]), HANDLE_VTABLE, obj)
            return Call(CALLS[name], [CAT, FOUND, FOLDER], regions, conv='t', ret='v')
        obj = handle_object(r, 0, r.choice(FOLDERS), regions) if r.random() < 0.5 else 0
        struct.pack_into('<2I', regions[CAT], 0x30, HANDLE_VTABLE, obj)
        regions[OUT] = r.bytes(8)
        return Call(CALLS[name], [CAT, OUT, FOLDERTEXT], regions, conv='t', ret='a')
    if name == 'findbank':   # deque B's banks (gaps now and then), each a name and a Handle on a folder; the folder and path asked for often a bank's
        n = r.choice([0, 1, 2, 3, 5])
        folders, names = [r.choice(FOLDERS) for _ in range(n)], [r.choice(NAMES_GIVEN) for _ in range(n)]
        regions = {}
        objs = [0 if r.random() < 0.15 else bank(r, i, names[i], folders[i], regions) for i in range(n)]
        regions.update(category(r, banks, bank_objects=objs))
        regions[LOG] = log_region()
        files(r, regions[LOG], regions)
        k = r.randrange(n) if n and r.random() < 0.8 else None
        text = folders[k] if k is not None and r.random() < 0.9 else r.choice(FOLDERS)
        path = names[k] if k is not None and r.random() < 0.8 else r.choice(NAMES_GIVEN)
        regions[FOLDERTEXT] = text + b'\0' + r.bytes(8)
        regions[FOLDER] = string(r, text, FOLDERTEXT)
        regions[IMAGE] = path + b'\0' + r.bytes(8)
        obj = handle_object(r, 7, r.choice(FOLDERS), regions) if r.random() < 0.2 else 0
        regions[FOUND] = struct.pack('<B3x2I', r.choice([0, 1]), HANDLE_VTABLE, obj)
        regions[OUT] = r.bytes(8)
        return Call(CALLS[name], [CAT, OUT, IMAGE, FOLDER, FOUND], regions, conv='t', ret='b')
    if name in ('readbankmap', 'loadbankmap'):   # BANK.map's bytes in turn (seq_read) over banks already loaded, whose folders and names it often names
        count = r.choice([0, 1, 2, 3, 4])
        folder = r.choice(FOLDERS[:4])
        paths = [r.choice([b'Bank0.bnk', b'sub\\Bank1.bnk', b'x', b'a\\b\\c.bnk']) for _ in range(count)]
        flags = [r.choice([0, 0, 0x20, 0x21, r.getrandbits(8)]) for _ in range(count)]
        entries = b''.join(r.bytes(9) + bytes([flags[i]]) + r.bytes(1) for i in range(count))
        guid = r.choice([BANK_GUID, BANK_GUID, BANK_GUID, SFX_GUID, BANK_GUID[:12] + r.bytes(4), BANK_GUID[:8] + r.bytes(4) + BANK_GUID[12:]])   # (one dword off)
        data = guid + r.bytes(4) + r.bytes(4) + struct.pack('<I', count) + entries
        data += b''.join(struct.pack('<I', len(p)) + p for p in paths) + bytes(4)
        n = r.choice([0, 1, 2, 3])
        names = [r.choice([p.split(b'\\')[-1] for p in paths] + [b'y']) for _ in range(n)]
        folders = [r.choice([folder, folder, b'C:\\tp\\Global\\', r.choice(FOLDERS)]) for _ in range(n)]
        regions = {}
        objs = [0 if r.random() < 0.15 else bank(r, i, names[i], folders[i], regions, recorders=False) for i in range(n)]
        regions.update(category(r, banks, bank_objects=objs))
        log = log_region()
        regions[LOG] = log
        files(r, log, regions, wad=name == 'readbankmap')
        regions[FILE] = data
        regions[FOLDERTEXT] = folder + b'\0' + r.bytes(8)
        regions[SOUND_SYSTEM] = struct.pack('<I', SOUND)
        sound = bytearray(r.bytes(0x44))
        root = r.choice([b'C:\\tp\\', b''])
        sound[0x30:0x44] = string(r, root, SOUNDTEXT)
        regions[SOUND] = sound
        regions[SOUNDTEXT] = root + b'\0' + r.bytes(8)
        regions[OUT] = r.bytes(8)
        if name == 'readbankmap':
            seq_read(log, 2, FILE, len(data), buffer=0, count=1)
            strings(log, 2, 0, out=1)
            set_return(log, 5, len(data))
            set_return(log, 6, 0x1c)
            regions[STREAM] = struct.pack('<2I', VT_STREAM, r.getrandbits(32))
            regions[VT_STREAM] = vtable({2: 2})
            return Call(CALLS[name], [CAT, OUT, STREAM, FOLDERTEXT], regions, conv='t', ret='a')
        # the category's Handle (+0x30) opens the stream on SRC (its slot 3) and gives its folder's path (its slot 4, recorder 20: a String filled in)
        vt = vtable({3: 5})
        struct.pack_into('<I', vt, 0x10, recorder(20, 3))
        regions[VT_HANDLE] = vt
        struct.pack_into('<2I', regions[CAT], 0x30, VT_HANDLE, r.getrandbits(32))
        if r.random() < 0.3:   # (the status the open answers through: negative, or 0 / 1 at the edge)
            set_return(log, 3, NEGATIVE)
            regions[NEGATIVE] = struct.pack('<i', r.choice([-1, -0x80000000, 0, 0, 1]))
        else:
            echo(log, 3, 0)
        fill(log, 3, 1, struct.pack('<2I', STREAM_VTABLE, SRC), 0)
        strings(log, 3, 4, out=3)
        fill(log, 20, 1, string(r, folder, PATHSTR), 1)
        strings(log, 20, 0, out=7)
        regions[PATHSTR] = folder + b'\0' + r.bytes(8)
        seq_read(log, 1, FILE, len(data), buffer=0, count=1)
        strings(log, 1, 0, out=1)
        set_return(log, 4, len(data))
        set_return(log, 5, 0x1c)
        regions[VT_SRC] = vtable({0: 1, 1: 2})
        regions[SRC] = struct.pack('<3I', VT_SRC, 0, r.choice([1, 2]))
        regions[STREAM] = b'cat_xBANK.map\0'
        return Call(CALLS[name], [CAT, OUT, STREAM, r.getrandbits(32)], regions, conv='t', ret='a')
    if name == 'freecat':
        regions = category(r, banks, holes=True)
        nrec = struct.unpack_from('<I', regions[CAT], 0x64)[0]
        struct.pack_into('<I', regions[CAT], 0x64, r.choice([nrec, nrec, max(0, nrec - 1), 0]))
        return Call(CALLS[name], [CAT], regions, conv='t', ret='a')
    if name == 'freebank':   # deque B of bank objects (any non-zero) and gaps; a long run crosses into the second block
        n = r.choice([0, 1, 3, 6, r.randrange(0x3f0, 0x401)])
        objs = [r.choice([0, RAM + 0x100, r.getrandbits(32) | 1, r.getrandbits(32) | 1]) if r.random() < 0.3 else RAM + 4 * i + 4 for i in range(n)]
        regions = category(r, banks, bank_objects=objs)
        return Call(CALLS[name], [CAT], regions, conv='t', ret='a')
    if name == 'entries':   # the rest of BANK.map: the 11-byte entries, then each path (a u32 length and the chars)
        count = r.choice([0, 1, 2, 3, 5])
        paths = [bytes(r.choice(b'ab\\/.') for _ in range(r.randrange(1, 12))) for _ in range(count)]
        data = r.bytes(11 * count) + b''.join(struct.pack('<I', len(p)) + p for p in paths) + r.bytes(r.choice([0, 0, 3])) + bytes(4)
        regions = {HEAP: heap(0x2000), CAT: r.bytes(0xd0), FILE: data}
        log = log_region()
        regions[LOG] = log
        pos = r.choice([0x1c, 0x1c, 0, 0x100])
        seq_read(log, 2, FILE, len(data), buffer=0, count=1)
        strings(log, 2, 0, out=1)
        set_return(log, 5, len(data) + pos)
        set_return(log, 6, pos)
        regions[STREAM] = struct.pack('<2I', VT_STREAM, r.getrandbits(32))
        regions[VT_STREAM] = vtable({2: 2})
        regions[OUT] = struct.pack('<2I', r.getrandbits(32), count)   # (a count the file does not hold sends the walk through a length read from an entry: wild)
        return Call(CALLS[name], [CAT, STREAM, OUT], regions, conv='t', ret='v')
    if name == 'fixup':
        top = r.choice([1, 1, 2])
        body, top = image(r, banks, 'c', top)
        regions[IMAGE] = body
        regions[OUT] = r.bytes(4)
        return Call(CALLS[name], [CAT, IMAGE, OUT, top if r.random() < 0.9 else 0], regions, conv='t', ret='a')
    level = {'record': 'c', 'sound': 's', 'variation': 'v'}[name]
    body, _top = image(r, banks, level, 1)
    size = {'c': 0x18, 's': 0x14, 'v': 0x2a}[level]
    regions[IMAGE] = body[size:]
    regions[OUT] = body[:size] + r.bytes(0x10)   # the record itself apart from the data that follows it
    return Call(CALLS[name], [CAT, IMAGE, OUT], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
