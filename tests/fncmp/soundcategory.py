# A sound category's SFX.map loading (src/game/core/soundcategory) against the originals: the image fixups (0x6c0510 and below: a valid image of
# 1 or 2 categories, 0-3 sounds each, 0-3 variations, 0-3 samples and 0-3 links, built in the order the fixup walks it; the samples' banks 0 or
# within deque C, the weights random, delta-coded or not) over a category object whose deques A (the category records, each with a PtrList of
# 0-3 banks) and C (the bank indices) are small stretches of one or two blocks (the block boundary crossed now and then); a variation's sample
# count (0x6bd7a0) and the simple sound mark (0x6c0550: each of its conditions met or missed). The free slots in deques A and B (0x6bf150 with
# holes in A, 0x6bf1c0 over bank objects and gaps, long runs crossing into the second block); the BANK.map entries read (0x6bf9e0) through a stream recorder whose reads hand out the file's bytes in turn (seq_read).
# The constructor (0x6beb20) and the singleton's getter (0x6bead0): see case_ctor(). The sample's indices (0x6d1bd0) and deque A's record
# (0x6d1dc0): see locate_case(). The banks' resources released (0x6b9660, 0x6b9760): see case_release(). The record's destructor (0x6bea00): case_unload()'s record on the singleton.
# The sound object's category load (0x6b8720): case_load(obj=True); its record banks' release (0x6b8570): case_release()'s releaserec. A bank's flag 0x40 (0x6b9270): case_release()'s deque B.
# The variation's flagged samples moved last (0x6c0bd0): case_release()'s deque B. A record released (0x6c0850): locate_case().
# A command's sound found in an SFX.map sound table (0x6c5870): 'findsound' in case(). Every variation's flagged samples hidden or shown again (0x6c0c90): case_hide(),
# also through a channel's switch (0x6b92f0: its byte +0x64 the same as the argument's low byte or not).
# usage: soundcategory.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,...)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import fileopen  # noqa: E402  (the file library's ranges, imports and globals)
from fncmp import HEAP, IMPORT_TABLE, LOG, RAM, Bench, Call, Mask, _cdecl_log, echo, fill, fill_arg, heap, import_table, log_region, recorder, seq_read, seq_seek, set_return, strings, vtable  # noqa: E402

CALLS = {'count': 0x6bd7a0, 'simple': 0x6c0550, 'variation': 0x6c06e0, 'sound': 0x6c0610, 'record': 0x6c05b0, 'fixup': 0x6c0510, 'readimage': 0x6c0360,
         'readmap': 0x6c0450, 'freecat': 0x6bf150, 'freebank': 0x6bf1c0, 'entries': 0x6bf9e0, 'bankdtor': 0x6c01b0, 'bankdel': 0x6c0190,
         'openfolder': 0x6bf850, 'setfolder': 0x6bf260, 'findbank': 0x6bfa60, 'readbankmap': 0x6bfe40, 'loadbankmap': 0x6c0210,
         'addref': 0x6338c0, 'release': 0x6338d0, 'load': 0x6bf330, 'listdtor': 0x6c2f40, 'listdel': 0x6c30c0, 'findrec': 0x6c0f70,
         'dtor': 0x6bee10, 'deldtor': 0x6bed80, 'singleton': 0x6beb10, 'unload': 0x6c08b0, 'ctor': 0x6beb20, 'get': 0x6bead0,
         'locate': 0x6d1bd0, 'recordat': 0x6d1dc0, 'releaseall': 0x6b9660, 'releaserec': 0x6b9760,
         'recdtor': 0x6bea00, 'objload': 0x6b8720, 'bankflag': 0x6b9270,
         'partition': 0x6c0bd0, 'recrelease': 0x6c0850, 'findsound': 0x6c5870, 'objrelease': 0x6b8570,
         'objrecrelease': 0x6ba090, 'hide': 0x6c0c90, 'chanhide': 0x6b92f0}
B = Bench([(0x6bd7a0, 0x6bd7ac), (0x6bf150, 0x6bf31c), (0x6bf850, 0x6c0303), (0x6c0310, 0x6c035f), (0x6c0360, 0x6c0844), (0x4036a0, 0x4036e0), (0x639100, 0x639120),
           (0x6b9b30, 0x6b9b94), (0x5e42d0, 0x5e431c), (0x6338b0, 0x633900), (0x6c1940, 0x6c1b00), (0x6c2990, 0x6c29c0), (0x4f6740, 0x4f67c0), (0x632200, 0x632230),
           (0x6bf330, 0x6bf850), (0x632150, 0x632380), (0x6c0860, 0x6c3200), (0x5f7980, 0x5f79cf), (0x67c250, 0x67c2a0),
           (0x6beb10, 0x6beb1a), (0x6bed80, 0x6bee0b), (0x6bee10, 0x6bf14c), (0x5ec210, 0x5ec220), (0x5f8240, 0x5f8310), (0x5f8420, 0x5f8460), (0x619800, 0x619870),
           (0x6beb20, 0x6bed76), (0x6bead0, 0x6beb03), (0x47db00, 0x47db30), (0x5f61a0, 0x5f61f0),
           (0x6d1bd0, 0x6d1db5), (0x6d1dc0, 0x6d1e2f), (0x6b9660, 0x6b9760), (0x6b9760, 0x6b984c), (0x6c0bd0, 0x6c0c87), (0x6c0c90, 0x6c0d8a), (0x6c0850, 0x6c08ad), (0x6bea00, 0x6beac5), (0x6b8720, 0x6b87c5), (0x6b8690, 0x6b8711), (0x6b9270, 0x6b92ef), (0x6c5870, 0x6c58ce), (0x6b8570, 0x6b859f), (0x6ba090, 0x6ba0be), (0x6b92f0, 0x6b930e)]
          + fileopen.RANGES,
          hooks={0x691de0: lambda e: e.r.__setitem__('EAX', e.r['ECX']), 0x691ea0: lambda e: None, 0x67a7d0: _cdecl_log})   # (the STL lock the map's insert takes: the port leaves it out)
CAT, OUT, RECS, NODES = RAM, RAM + 0x100, RAM + 0x200, RAM + 0x400
OBJREL = RAM + 0x2e00   # (the sound object of the record banks' release forward, 0x6b8570)
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
REC_VT, TREE_HEAD, TREE_NIL, TREE_NODE, NODE_TEXT, NAME_TEXT = RAM + 0xdc0, RAM + 0xe00, RAM + 0xe40, RAM + 0xe80, RAM + 0xf00, RAM + 0xf80
WAD = RAM + 0x50000   # a WAD holding <name>BANK.map and <name>SFX.map (SoundCategory_Load's)
WAD_FOLDERS = [b'C:\\tp\\data\\sfx\\', b'C:\\tp\\data\\sfx', b'D:\\snd\\data\\x\\', b'C:\\tp\\amb\\']
TREE_H_NIL = 0xfb2008   # (the map's nil node's pointer)
J_HEAD, J_NIL, SINGLETON = RAM + 0x1000, RAM + 0x1040, 0xfb1f28   # tree J's head and nil node (the destructor's); the singleton category
UNLOAD_BANKS, UNLOAD_EXTRAS, UNLOAD_VT = RAM + 0x1100, RAM + 0x1200, RAM + 0x1300   # the unload's banks, their +8 objects, their vtables
OBJ, OBJ_CHAN, OBJ_BASE, OBJ_PATH = RAM + 0x1e00, RAM + 0x1e40, RAM + 0x1f00, RAM + 0x1f40   # (objload: the sound object, its channel, texts)
ENTRIES, INFO, SFX_SOUNDS, SFX_VARS, SFX_SAMPLES = RAM + 0x1400, RAM + 0x1480, RAM + 0x1500, RAM + 0x1a00, RAM + 0x1c00   # (locate_case())
OPS = [n for n in CALLS if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]


def deque(r, at, mapat, values, regions):
    """A deque's begin iterator (16 bytes) over values starting at a random place in its first block at `at` (into the next, at + 0x1000 or + 0x3000,
    when they run past it); maps the stretch of each block they use and the map's two slots."""
    n = len(values)
    start = max(0, min(0x3ff, r.choice([0, 0x3ff - n // 2, 0x400 - n, r.randrange(max(1, 0x400 - n + 1))])))
    blocks = [at, at + r.choice([0x1000, 0x3000])]
    while len(blocks) <= (start + n - 1) // 0x400:   # (more than two blocks' worth: a block more each 1024 records, as far apart)
        blocks.append(blocks[-1] + r.choice([0x1000, 0x2000]))
    regions[mapat] = struct.pack('<%dI' % len(blocks), *blocks)
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
    blocks = struct.unpack('<%dI' % (len(regions[mapat]) // 4), regions[mapat])
    q = (cur - first) // 4 + len(values)
    b = q // 0x400 - 1 if q and q % 0x400 == 0 else q // 0x400
    return it + struct.pack('<4I', blocks[b], blocks[b] + 0x1000, blocks[b] + 4 * (q - 0x400 * b), mapat + 4 * b) + struct.pack('<3I', mapat, len(blocks), len(values))


def files(r, log, regions, wad=True):
    """The file library's world (fileopen.py's): the imports recorders and their answers, a WAD header for ReadFile, the globals, the heap. Without
    wad (the log's two fills taken), GetFileAttributesA always answers none: no WAD is found and no folder opens."""
    for imp, k in fileopen.K.items():
        if r.random() < 0.9:
            set_return(log, k, r.choice(ANSWERS.get(imp, [0, 1, r.getrandbits(32)])))
    if r.random() < 0.8:
        echo(log, fileopen.K['SetFilePointer'], 1)
    if wad:
        header = fileopen.wad_header(r)
        struct.pack_into('<I', header, 0x48, 0)   # (no entries: a record would be the header's bytes again, its name wild)
        fill(log, fileopen.K['ReadFile'], 1, bytes(header), 0)
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


def name_hash(text):
    """The WAD directory's name hash (src/game/files/wadfind name_hash): the upper-cased chars weighted 1, 2, 4, 8 in fours, the rest added."""
    up = [c - 0x20 if 0x61 <= c <= 0x7a else c for c in text]
    h = sum(sum(up[4 * g + i] << i for i in range(4)) for g in range(len(up) >> 2))
    return (h + sum(c & 0xff for c in up[len(up) & ~3:])) & 0xffffffff


def wad_image(r, files):
    """A WAD's bytes: the header (DWFB, version 0-2: 2 reads the names as one block), the 0x28-byte records (the name's hash, offset and length,
    the data's offset and size, unpacked), the names, then the files' data."""
    count = len(files)
    names_at = 0x58 + 0x28 * count
    names = b''.join(n + b'\0' for n, _ in files)
    data_at = names_at + len(names) + r.choice([0, 0, 4])
    out = bytearray(r.bytes(0x58))
    struct.pack_into('<2I', out, 0, 0x42465744, r.choice([0, 1, 2, 2]))
    struct.pack_into('<3I', out, 0x48, count, names_at, len(names))
    at_name, at_data, blob = names_at, data_at, b''
    for n, d in files:
        rec = bytearray(0x28)
        struct.pack_into('<7I', rec, 0, name_hash(n), at_name, len(n), at_data, len(d), 0, len(d))
        out += rec
        at_name += len(n) + 1
        at_data += len(d)
        blob += d
    out += names + bytes(data_at - names_at - len(names)) + blob
    return bytes(out)


def category(r, banks=4, holes=False, bank_objects=None, many=False):
    """The category object with deques A (records with bank lists) and C (bank indices); returns regions. With `many`, deque A holds more than
    1024 cells (three blocks or more): mostly gaps, the first record once in the second half, the others anywhere."""
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
    vals = [0 if holes and r.random() < 0.3 else RECS + 0x40 * i for i in range(nrec)]
    if many:
        n = r.randrange(0x401, 0xa00)
        vals = [RECS + 0x40 * r.randrange(1, nrec) if nrec > 1 and r.random() < 0.25 else 0 for i in range(n)]
        vals[r.randrange(n // 2, n)] = RECS
    cat[0x3c:0x68] = deque_full(r, BLOCKA, MAPA, vals, regions)
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


def case_dtor(r, name):
    """The destructor over deque A of 1-3 records (the first's vtable a recorder: its slot 0 logged), B of 0 to ~0x900 banks and C of 0 to
    ~0x900 indices (the boundaries crossed), the folder Handle on an object (count 1-3) or none, tree J empty (its counter 0xfb2004 1, more or 0)
    and map H empty or of one node (its counter 0xfb200c the same); deleting with flags bit 0 or not; the singleton's at 0xfb1f28."""
    n = r.choice([0, 1, 3, r.randrange(0x3f0, 0x401), r.randrange(0x401, 0x900)])
    regions = category(r, r.choice([0, 1, 4, r.randrange(0x3f0, 0x401), r.randrange(0x401, 0x900)]), bank_objects=[r.getrandbits(32) for _ in range(n)])
    cat = regions[CAT]
    recs = bytearray(regions[RECS])
    struct.pack_into('<I', recs, 0, REC_VT)
    regions[RECS] = recs
    regions[REC_VT] = vtable({}, 4)
    regions[LOG] = log_region()
    files(r, regions[LOG], regions, wad=False)   # (the imports: the Handle's release, InterlockedDecrement)
    obj = handle_object(r, 0, r.choice(FOLDERS), regions, recorders=False) if r.random() < 0.5 else 0
    struct.pack_into('<2I', cat, 0x30, HANDLE_VTABLE, obj)
    regions[J_HEAD] = struct.pack('<3I', J_HEAD, J_NIL, J_HEAD) + r.bytes(0x18)
    nil = bytearray(r.bytes(0x28))
    struct.pack_into('<I', nil, 0x20, 1)
    regions[J_NIL] = nil
    struct.pack_into('<4I', cat, 0x1c, r.getrandbits(32), J_HEAD, r.getrandbits(32), 0)
    nil = bytearray(0x28)
    struct.pack_into('<I', nil, 0x24, 1)
    regions[TREE_NIL] = nil
    if r.random() < 0.5:
        text = r.choice([b'x', WAD_FOLDERS[0] + b'cat_a', b''])
        node = bytearray(0x28)
        struct.pack_into('<4I', node, 0, TREE_NIL, TREE_HEAD, TREE_NIL, r.getrandbits(32))
        node[0x10:0x24] = string(r, text, NODE_TEXT)
        struct.pack_into('<I', node, 0x24, 1)
        regions[TREE_NODE] = node
        regions[NODE_TEXT] = text + b'\0' + r.bytes(8)
        regions[TREE_HEAD] = struct.pack('<3I', TREE_NODE, TREE_NODE, TREE_NODE) + bytes(0x1c)
        size = 1
    else:
        regions[TREE_HEAD] = struct.pack('<3I', TREE_HEAD, TREE_NIL, TREE_HEAD) + bytes(0x1c)
        size = 0
    struct.pack_into('<IIBxxxI', cat, 0xc, r.getrandbits(32), TREE_HEAD, 0, size)
    regions[0xfb2000] = struct.pack('<4I', J_NIL, r.choice([1, 1, 2, 0]), TREE_NIL, r.choice([1, 1, 2, 0]))
    if name == 'singleton':
        regions[SINGLETON] = regions.pop(CAT)
        return Call(CALLS[name], [], regions, conv='c', ret='v')
    if name == 'deldtor':
        return Call(CALLS[name], [CAT, r.choice([0, 1, 1, 3, 2])], regions, conv='t', ret='a')
    return Call(CALLS[name], [CAT], regions, conv='t', ret='v')


def case_unload(r, name='unload'):
    """A record unloaded (0x6c08b0): deque B of 7-10 banks (now and then 0), each a recorder object of its own slot-0 recorder (1-10: answering
    0 mostly, else 1 or 2) whose +8 is another (12-21) or null; a record of deque A (its bank list of 0-3 indices 1-6), at its own index
    (+0x20) or another, or none; map H empty or of one node (its key the index or another), its size 0 to 5 (StdMapH_ClearAt's bound)."""
    nb = r.choice([7, 7, 10])
    objs = [0 if r.random() < 0.2 else UNLOAD_BANKS + 0x10 * i for i in range(nb)]
    regions = category(r, 4, bank_objects=objs)
    log = log_region()
    regions[LOG] = log
    for i in range(nb):
        extra = UNLOAD_EXTRAS + 0x10 * i if r.random() < 0.6 else 0
        regions[UNLOAD_BANKS + 0x10 * i] = struct.pack('<3I', UNLOAD_VT + 4 * i, r.getrandbits(32), extra) + r.bytes(4)
        regions[UNLOAD_EXTRAS + 0x10 * i] = struct.pack('<I', UNLOAD_VT + 0x40 + 4 * i) + r.bytes(0xc)
        set_return(log, 1 + i, r.choice([0, 0, 0, 1, 2]))
    regions[UNLOAD_VT] = struct.pack('<10I', *[recorder(1 + i, 0) for i in range(10)])
    regions[UNLOAD_VT + 0x40] = struct.pack('<10I', *[recorder(12 + i, 0) for i in range(10)])
    cat = regions[CAT]
    nrec = struct.unpack_from('<I', cat, 0x64)[0]
    j = r.randrange(nrec)
    recs = bytearray(regions[RECS])
    for i in range(nrec):
        struct.pack_into('<I', recs, 0x40 * i + 0x20, i if r.random() < 0.85 else r.randrange(4))
    regions[RECS] = recs
    nil = bytearray(0x28)
    struct.pack_into('<I', nil, 0x24, 1)
    regions[TREE_NIL] = nil
    regions[TREE_H_NIL] = struct.pack('<I', TREE_NIL)
    if r.random() < 0.5:
        text = r.choice([b'x', b'', b'cat_b2'])
        node = bytearray(0x28)
        struct.pack_into('<4I', node, 0, TREE_NIL, TREE_HEAD, TREE_NIL, r.choice([j, j, r.randrange(4)]))
        node[0x10:0x24] = string(r, text, NODE_TEXT)
        struct.pack_into('<I', node, 0x24, 1)
        regions[TREE_NODE] = node
        regions[NODE_TEXT] = text + bytes(1) + r.bytes(8)
        regions[TREE_HEAD] = struct.pack('<3I', TREE_NODE, TREE_NODE, TREE_NODE) + bytes(0x1c)
    else:
        regions[TREE_HEAD] = struct.pack('<3I', TREE_HEAD, TREE_NIL, TREE_HEAD) + bytes(0x1c)
    struct.pack_into('<IIBxxxI', cat, 0xc, r.getrandbits(32), TREE_HEAD, 0, r.choice([0, 1, 2, 3, 5]))
    regions[OUT] = r.bytes(4)
    if name == 'recdtor':   # (the record's destructor: the category the singleton; the record's +8 a block or null, +0xc a String)
        recs = bytearray(regions[RECS])
        struct.pack_into('<I', recs, 0x40 * j + 8, r.choice([0, HEAP + 0x1000]))
        recs[0x40 * j + 0xc:0x40 * j + 0x20] = string(r, b'cat_x', NAMES)
        regions[RECS], regions[NAMES] = recs, b'cat_x\0' + r.bytes(10)
        regions[SINGLETON] = regions.pop(CAT)
        regions[0xfb1ff8] = bytes([r.choice([1, r.getrandbits(8) | 1])])
        return Call(CALLS[name], [RECS + 0x40 * j], regions, conv='t', ret='v')
    return Call(CALLS['unload'], [CAT, OUT, RECS + 0x40 * j if r.random() < 0.95 else 0], regions, conv='t', ret='a')


def case_ctor(r, name):
    """The constructor over random bytes: tree J's and map H's nil nodes made already or not (their counters any), the heap failing now
    and then (a failed record faults in both). The allocator and comparator bytes it copies from its uninitialised locals (+0xc, +0xd, +0x1c,
    +0x1d and the deques' +0x38, +0x68, +0x98) are masked: the listing's only reads of them are those copies (0x6beb5d / 0x6beb61 /
    0x6beb86 / 0x6bebb0 / 0x6bebea and StdMapH_Ctor's two at 0x6c124b / 0x6c1253 read the stack, never the object), and the emulator fails a
    case whose code reads one."""
    nil_j, nil_h = bytearray(r.bytes(0x28)), bytearray(0x28)
    struct.pack_into('<I', nil_j, 0x20, 1)
    struct.pack_into('<I', nil_h, 0x24, 1)
    regions = {CAT: r.bytes(0xd0), J_NIL: nil_j, TREE_NIL: nil_h, HEAP: heap(0x2000, fail=r.choice([0] * 8 + [r.randrange(1, 10)])),
               0xfb2000: struct.pack('<4I', r.choice([0, J_NIL]), r.getrandbits(32), r.choice([0, TREE_NIL]), r.getrandbits(32))}
    mask = Mask(CAT, [0xc, 0xd, 0x1c, 0x1d, 0x38, 0x68, 0x98], 'the allocator and comparator bytes: the original copies its uninitialised '
                'stack bytes there (the port 0); nothing reads them')
    if name == 'get':   # (the singleton: made unless the guard's bit 0 is set (its other bits any); atexit a recorder)
        regions[0xfb1ff8] = bytes([r.choice([0, 1, r.getrandbits(8), r.getrandbits(8) & 0xfe])])
        regions[SINGLETON] = regions.pop(CAT)
        regions[LOG] = log_region()
        mask.region = SINGLETON
        return Call(CALLS['get'], [], regions, ret='a', masks=[mask])
    return Call(CALLS['ctor'], [CAT], regions, conv='t', ret='a', masks=[mask])


def locate_case(r, name):
    """The singleton's deque A (category(): records null now and then, over three blocks or more now and then; the guard's bit 0 set). Each
    record's SFX.map category (+8, null now and then) has 0-5 sounds (a wrapping count now and then) at SFX_SOUNDS + 0x100 * i, each sound
    0-5 variations at SFX_VARS (each 0-5 samples at SFX_SAMPLES). info's sound one of a record's (its end too), off the stride, outside them
    or null; its variation and sample about their bases (below them now and then) or null. recordat: an index under the count or past it."""
    regions = category(r, 4, holes=True, many=r.random() < 0.2)
    regions[SINGLETON] = regions.pop(CAT)
    regions[0xfb1ff8] = bytes([r.choice([1, r.getrandbits(8) | 1])])
    count = struct.unpack_from('<I', regions[SINGLETON], 0x64)[0]
    if name in ('recrelease', 'objrecrelease'):   # (the records' slot 0 recorder 0; an index under the count; the category the singleton's place)
        recs = bytearray(regions[RECS])
        for i in range(len(recs) // 0x40):
            struct.pack_into('<I', recs, 0x40 * i, REC_VT)
        log = log_region()
        set_return(log, 0, r.getrandbits(32))
        regions.update({RECS: recs, REC_VT: vtable({}, 4), LOG: log, OUT: r.bytes(4)})
        index = r.randrange(count) if count else 0
        if name == 'objrecrelease':   # (the sound object's forward: this unused; index 0 its -1)
            return Call(CALLS[name], [r.getrandbits(32), OUT, r.choice([index, index, 0])], regions, conv='t', ret='a')
        return Call(CALLS[name], [SINGLETON, OUT, index], regions, conv='t', ret='a')
    if name == 'recordat':   # (a record in the cell past the last: only the count check keeps it out)
        first, _last, cur, _node = struct.unpack_from('<4I', regions[SINGLETON], 0x3c)
        blocks = struct.unpack('<%dI' % (len(regions[MAPA]) // 4), regions[MAPA])
        q = (cur - first) // 4 + count
        if q // 0x400 < len(blocks):
            regions[blocks[q // 0x400] + 4 * (q % 0x400)] = struct.pack('<I', RECS)
        return Call(CALLS[name], [r.getrandbits(32), r.choice([0, 1, 2, count, r.randrange(count + 3), r.getrandbits(32)]), r.getrandbits(32)], regions,
                    conv='t', ret='a')
    recs = bytearray(regions[RECS])
    entries = bytearray(r.bytes(0x40))
    counts = []
    for i in range(len(recs) // 0x40):
        counts.append(r.choice([0, 1, 2, 3, 5, 0x0ccccccd]))
        struct.pack_into('<II', entries, 8 * i, counts[-1], SFX_SOUNDS + 0x100 * i)
        struct.pack_into('<I', recs, 0x40 * i + 8, r.choice([ENTRIES + 8 * i] * 5 + [0]))
    regions[RECS], regions[ENTRIES] = recs, entries
    sounds = bytearray(r.bytes(0x100 * len(counts)))
    for o in range(0, len(sounds) - 0x14, 0x14):
        struct.pack_into('<II', sounds, o + 4, r.choice([0, 1, 2, 5]), SFX_VARS)
    regions[SFX_SOUNDS] = sounds
    variations = bytearray(r.bytes(0x200))
    for o in range(0x40, 0x200 - 0x2a, 0x2a):
        struct.pack_into('<H', variations, o, r.choice([0, 1, 2, 5]))
        struct.pack_into('<I', variations, o + 8, SFX_SAMPLES)
    regions[SFX_VARS - 0x40] = variations
    regions[SFX_SAMPLES - 0x40] = r.bytes(0x100)
    i = r.randrange(len(counts))
    sound = r.choice([SFX_SOUNDS + 0x100 * i + 0x14 * r.randrange(min(counts[i], 5) + 1), SFX_SOUNDS + 0x100 * i + r.randrange(0x64),
                      SFX_SOUNDS + 0x100 * i + 0x14 * r.randrange(6), RAM + 0x1490, 0, SFX_SOUNDS + 0x100 * i + 0x14 * min(counts[i], 5)])   # (its end too)
    variation = r.choice([SFX_VARS + 0x2a * r.randrange(6), SFX_VARS + r.randrange(-0x2a, 0x100), 0])
    sample = r.choice([SFX_SAMPLES + 0x10 * r.randrange(6), SFX_SAMPLES + r.randrange(-0x20, 0x80), 0])
    regions[INFO] = struct.pack('<4I', r.getrandbits(32), sound, variation, sample) + r.bytes(0x10)
    regions[OUT] = r.bytes(0x10)
    return Call(CALLS[name], [r.getrandbits(32), OUT, OUT + 4, OUT + 8, OUT + 12, INFO], regions, conv='t', ret='v')


def case_release(r, name):
    """The banks' resources released: the singleton's deque B of 0-10 banks (null now and then), each bank's +8 a resource whose slot 0 is a
    recorder (12-21; null now and then: the call faults in both). releaserec: a record or null (Fn_6c0200 gives no list: only *out)."""
    nb = r.choice([0, 1, 3, 7, 10])
    objs = [0 if r.random() < 0.2 else (BANKS + 0x40 * i if name in ('bankflag', 'partition') else UNLOAD_BANKS + 0x10 * i) for i in range(nb)]
    regions = category(r, 4, bank_objects=objs)
    regions[SINGLETON] = regions.pop(CAT)
    regions[0xfb1ff8] = bytes([r.choice([1, r.getrandbits(8) | 1])])
    regions[LOG] = log_region()
    for i in range(nb):
        regions[UNLOAD_BANKS + 0x10 * i] = struct.pack('<3I', r.getrandbits(32), r.getrandbits(32), UNLOAD_EXTRAS + 0x10 * i if r.random() < 0.97 else 0) + r.bytes(4)
        regions[UNLOAD_EXTRAS + 0x10 * i] = struct.pack('<I', UNLOAD_VT + 0x40 + 4 * i) + r.bytes(0xc)
    regions[UNLOAD_VT + 0x40] = struct.pack('<10I', *[recorder(12 + i, 0) for i in range(10)])
    if name == 'releaseall':
        return Call(CALLS[name], [], regions, ret='v')
    if name in ('bankflag', 'partition'):   # (the banks' flags +0x20 any; an index under the count or not)
        for i in range(nb):
            regions[BANKS + 0x40 * i] = r.bytes(0x30)
    if name == 'partition':   # (a variation of 0-9 samples (the high word random) at SFX_SAMPLES, their banks any)
        n = r.choice([0, 1, 2, 3, 5, 9])
        regions[0xfb1ff8] += r.bytes(3) + struct.pack('<I', r.getrandbits(32))
        regions[SFX_VARS] = struct.pack('<HHII', n, r.getrandbits(16), r.getrandbits(32), SFX_SAMPLES) + r.bytes(0x1e)
        samples = bytearray(r.bytes(0x10 * n + 0x20))
        for i in range(n + 2):
            struct.pack_into('<H', samples, 0x10 * i + 0xc, r.choice([r.randrange(nb + 2), r.randrange(nb + 2), r.getrandbits(16)]))
        regions[SFX_SAMPLES - 0x10] = bytes(samples)
        return Call(CALLS[name], [r.getrandbits(32), SFX_VARS], regions, conv='t', ret='v')
    if name == 'bankflag':
        return Call(CALLS[name], [r.getrandbits(32), r.choice([0, 1, 2, r.randrange(nb + 2), r.getrandbits(32)])], regions, conv='t', ret='b')
    regions[OUT] = r.bytes(4)
    if name == 'objrelease':   # (the sound object's forward: its channel +0x24 (not read) or none)
        regions[OBJREL] = r.bytes(0x24) + struct.pack('<I', r.choice([r.getrandbits(32) | 1] * 4 + [0]))
        return Call(CALLS[name], [OBJREL, OUT, r.choice([RECS, RECS, 0])], regions, conv='t', ret='a')
    return Call(CALLS[name], [OUT, r.choice([RECS, 0])], regions, conv='s', ret='a')


HIDE_BLOCK = RAM + 0x70000   # (case_hide(): deque A's block before its first, through the map slot before MAPA)
HIDE_CHAN = RAM + 0x2f00   # (case_hide(): the channel of the switch 0x6b92f0)


def case_hide(r, name):
    """0x6c0c90 on the singleton: deque A's records (null now and then, over three blocks or more now and then), each one's SFX.map category (+8,
    null now and then) 0-3 sounds (a wrapping count now and then) at SFX_SOUNDS + 0x100 * i, each 0-3 variations at SFX_VARS (shared: walked
    again from each sound), each 0-5 samples (the high word random) at SFX_SAMPLES whose banks are mostly deque B's 0-7 (flags any; null now
    and then); hide's low byte 0 or not (the rest random)."""
    nb = r.choice([0, 1, 3, 7])
    objs = [0 if r.random() < 0.2 else BANKS + 0x40 * i for i in range(nb)]
    regions = category(r, 4, holes=True, bank_objects=objs, many=r.random() < 0.15)
    regions[SINGLETON] = regions.pop(CAT)
    regions[0xfb1ff8] = bytes([r.choice([1, r.getrandbits(8) | 1])]) + r.bytes(3) + struct.pack('<I', r.getrandbits(32))
    for i in range(nb):
        regions[BANKS + 0x40 * i] = r.bytes(0x30)
    if r.random() < 0.15:   # (the begin iterator's block start past its position: the first cells' offsets negative, block -1 through the map slot before it)
        cat = bytearray(regions[SINGLETON])
        struct.pack_into('<I', cat, 0x3c, struct.unpack_from('<I', cat, 0x44)[0] + 4 * r.choice([r.randrange(1, 0x401), 0x3ff, 0x400]))
        regions[SINGLETON] = cat
        regions[MAPA - 4] = struct.pack('<I', HIDE_BLOCK) + regions.pop(MAPA)
        regions[HIDE_BLOCK] = struct.pack('<1024I', *[r.choice([0, 0, RECS, RECS + 0x40, RECS + 0x80]) for _ in range(0x400)])
    recs = bytearray(regions[RECS])
    entries = bytearray(r.bytes(0x40))
    for i in range(len(recs) // 0x40):
        struct.pack_into('<II', entries, 8 * i, r.choice([0, 1, 2, 3, 3, 0x0ccccccd]) if r.random() < 0.1 else r.choice([0, 1, 2, 3]),
                         SFX_SOUNDS + 0x100 * i)
        struct.pack_into('<I', recs, 0x40 * i + 8, r.choice([ENTRIES + 8 * i] * 5 + [0]))
    regions[RECS], regions[ENTRIES] = recs, entries
    sounds = bytearray(r.bytes(0x100 * (len(recs) // 0x40)))
    for o in range(0, len(sounds), 0x100):   # (each record's at + 0x100 * i: four sounds, past the counts too)
        for j in range(4):
            struct.pack_into('<II', sounds, o + 0x14 * j + 4, r.choice([0, 1, 2, 3]), SFX_VARS)
    regions[SFX_SOUNDS] = sounds
    variations = bytearray(r.bytes(0x100))
    for o in range(0, 0x2a * 3, 0x2a):
        struct.pack_into('<HHI', variations, o, r.choice([0, 1, 2, 5]), r.getrandbits(16), r.getrandbits(32))
        struct.pack_into('<I', variations, o + 8, SFX_SAMPLES)
    regions[SFX_VARS] = variations
    samples = bytearray(r.bytes(0x60))
    for i in range(6):
        struct.pack_into('<H', samples, 0x10 * i + 0xc, r.choice([r.randrange(nb + 2), r.randrange(nb + 2), r.getrandbits(16)]))
    regions[SFX_SAMPLES] = samples
    hide = r.choice([0, 1, 1, 0x100, r.getrandbits(32)])
    if name == 'chanhide':
        chan = bytearray(r.bytes(0x70))
        chan[0x64] = r.choice([hide & 0xff, hide & 0xff, 0, 1, r.getrandbits(8)])
        regions[HIDE_CHAN] = chan
        return Call(CALLS[name], [HIDE_CHAN, hide], regions, conv='t', ret='v')
    return Call(CALLS['hide'], [SINGLETON, hide], regions, conv='t', ret='v')


def case(r):
    name = r.deal(OPS)
    if name in ('releaseall', 'releaserec', 'bankflag', 'partition', 'objrelease'):
        return case_release(r, name)
    if name in ('locate', 'recordat', 'recrelease', 'objrecrelease'):
        return locate_case(r, name)
    if name in ('ctor', 'get'):
        return case_ctor(r, name)
    if name in ('hide', 'chanhide'):
        return case_hide(r, name)
    if name in ('dtor', 'deldtor', 'singleton'):
        return case_dtor(r, name)
    if name in ('unload', 'recdtor'):
        return case_unload(r, name)
    if name == 'findsound':   # a table (null now and then) of 0-5 entries, its count theirs or fewer; ids small, the command's (its slot 2's answer) mostly; +4 0 now and then; slot 1 answering 0 now and then
        n = r.choice([0, 1, 2, 3, 5])
        want = r.choice([0, 1, 2, 3, r.getrandbits(32)])
        ents = bytearray(r.bytes(0x14 * max(n, 1)))
        for i in range(n):
            struct.pack_into('<II', ents, 0x14 * i, r.choice([want, want, r.randint(0, 3), r.getrandbits(32)]), r.choice([0, RECS, r.getrandbits(32)]))
        log = log_region()
        set_return(log, 1, r.choice([0, 1, 1, 1, r.getrandbits(32)]))
        set_return(log, 2, want)
        regions = {OUT: struct.pack('<II', r.choice([n, n, r.randint(0, n)]), NODES), NODES: ents, CAT: struct.pack('<I', REC_VT) + r.bytes(0x1c),
                   REC_VT: vtable({}, 4), LOG: log}
        return Call(CALLS[name], [r.getrandbits(32), r.choice([OUT, OUT, OUT, 0]), CAT], regions, conv='t', ret='a')
    if name == 'findrec':   # deque A's records (null now and then): +8 0 now and then, +0xc a String over a name; one of those names or another
        regions = category(r, 4, holes=True, many=r.random() < 0.3)   # (now and then over three blocks or more)
        names = [r.choice(NAMES_GIVEN + [b'Bank0', b'ab']) for _ in range(4)]
        recs = bytearray(regions[RECS])
        for i in range(len(recs) // 0x40):
            struct.pack_into('<I', recs, 0x40 * i + 8, r.choice([0, 1, r.getrandbits(32)]))
            recs[0x40 * i + 0xc:0x40 * i + 0x20] = string(r, names[i], NAMES + 0x40 * i)
            regions[NAMES + 0x40 * i] = names[i] + b'\0' + r.bytes(8)
        regions[RECS] = recs
        text = r.choice(names + [b'', b'Bank', b'x'])
        regions[FOLDERTEXT] = text + b'\0'
        return Call(CALLS[name], [CAT, r.choice([FOLDERTEXT] * 9 + [0])], regions, conv='t', ret='a')
    if name in ('listdtor', 'listdel'):   # a record's bank list: 0 to 5 nodes (+0 the item, +4 the next; the tail +8 any), each freed
        n = r.choice([0, 1, 2, 5])
        nodes = bytearray(r.bytes(0x10 * max(n, 1)))
        for i in range(n):
            struct.pack_into('<I', nodes, 0x10 * i + 4, NODES + 0x10 * (i + 1) if i + 1 < n else 0)
        regions = {HEAP: heap(), NODES: nodes, CAT: struct.pack('<4I', r.getrandbits(32), NODES if n else 0, r.getrandbits(32), r.getrandbits(32))}
        if name == 'listdtor':
            return Call(CALLS[name], [CAT], regions, conv='t', ret='v')
        return Call(CALLS[name], [CAT, r.choice([0, 1, 1, 3, 2])], regions, conv='t', ret='a')
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
    if name in ('load', 'objload'):   # a category loaded from a WAD in its folder (its BANK.map and SFX.map, read where the reads seek: seq_read + seq_seek)
        return case_load(r, banks, name == 'objload')
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


def case_load(r, banks, obj=False):
    """SoundCategory_Load over a category whose map (+0xc) holds 0 or 1 loaded names (the one asked for now and then: its record AddRef'd), whose
    banks mostly share the WAD's folder (so a BANK.map entry finds its folder open), the folder's WAD (GetFileAttributesA a file, mostly) holding the
    two maps (one now and then missing, or another GUID), the name sometimes empty or none."""
    folder = r.choice(WAD_FOLDERS)
    wad_path = (folder[:-1] if folder.endswith(b'\\') else folder) + b'.wad'   # (SoundCategory_OpenBankFolder's)
    name = r.choice([b'cat_a', b'cat_a', b'cat_b2', b'', None])
    root = r.choice([b'C:\\tp\\', b''])
    shared = [wad_path + b'*', root + b'Global\\']
    n = r.choice([0, 1, 2, 3])
    regions = {}
    objs = [bank(r, i, r.choice([b'Bank0.bnk', b'y']), r.choice(shared + [shared[0], r.choice(FOLDERS)]), regions, recorders=False) for i in range(n)]
    regions.update(category(r, banks, bank_objects=objs))
    cat = regions[CAT]
    nrec = struct.unpack_from('<I', cat, 0x64)[0]
    recs = regions[RECS]
    for i in range(nrec):
        struct.pack_into('<I', recs, 0x40 * i, REC_VT)
    regions[REC_VT] = vtable({}, 4)
    # the map: empty, or one node {key (an index in deque A), the String folder + name (or another)}
    struct.pack_into('<I', cat, 0x30, HANDLE_VTABLE)
    struct.pack_into('<I', cat, 0x34, 0)
    nil = bytearray(0x28)
    struct.pack_into('<I', nil, 0x24, 1)
    regions[TREE_NIL] = nil
    regions[TREE_H_NIL] = struct.pack('<I', TREE_NIL)
    if r.random() < 0.4:
        text = folder + (name or b'') if r.random() < 0.6 else r.choice([b'x', folder])
        node = bytearray(0x28)
        struct.pack_into('<4I', node, 0, TREE_NIL, TREE_HEAD, TREE_NIL, r.randrange(nrec))
        node[0x10:0x24] = string(r, text, NODE_TEXT)
        struct.pack_into('<I', node, 0x24, 1)
        regions[TREE_NODE] = node
        regions[NODE_TEXT] = text + b'\0' + r.bytes(8)
        regions[TREE_HEAD] = struct.pack('<3I', TREE_NODE, TREE_NODE, TREE_NODE) + bytes(0x1c)
        size = 1
    else:
        regions[TREE_HEAD] = struct.pack('<3I', TREE_HEAD, TREE_NIL, TREE_HEAD) + bytes(0x1c)
        size = 0
    struct.pack_into('<IIBxxxI', cat, 0xc, r.getrandbits(32), TREE_HEAD, 0, size)
    # BANK.map: entries whose paths banks hold now and then, flags 0x20 (Global) now and then
    count = r.choice([0, 1, 2, 3])
    paths = [r.choice([b'Bank0.bnk', b'sub\\Bank1.bnk', b'y']) for _ in range(count)]
    flags = [r.choice([0, 0, 0x20, 0x21]) for _ in range(count)]
    entries = b''.join(r.bytes(9) + bytes([flags[i]]) + r.bytes(1) for i in range(count))
    guid = BANK_GUID if r.random() < 0.92 else SFX_GUID
    bank_map = guid + r.bytes(4) + r.bytes(4) + struct.pack('<I', count) + entries + b''.join(struct.pack('<I', len(p)) + p for p in paths) + bytes(4)
    top = r.choice([0, 1, 1, 2])
    body, top = image(r, banks, 'c', top)
    guid = SFX_GUID if r.random() < 0.92 else BANK_GUID
    sfx_map = guid + struct.pack('<4I', r.choice([0, 0, 1]), 0, top, 0)[:12] + bytes(body)
    stem = (name or b'').lower()
    files_ = [(stem + b'bank.map', bank_map), (stem + b'sfx.map', sfx_map)]
    if r.random() < 0.15:
        files_.pop(r.randrange(2))
    if r.random() < 0.3:
        files_.insert(r.randrange(len(files_) + 1), (b'other.txt', r.bytes(r.randrange(1, 9))))
    wad = wad_image(r, files_)
    log = log_region()
    regions[LOG] = log
    files(r, log, regions, wad=False)
    regions[HEAP] = heap(0x18000)
    set_return(log, fileopen.K['GetFileAttributesA'], r.choice([0x20] * 8 + [0x10, 0xffffffff]))
    set_return(log, fileopen.K['CreateFileA'], r.choice([0x77] * 9 + [0xffffffff]))
    set_return(log, fileopen.K['ReadFile'], 1)
    set_return(log, fileopen.K['GetFileSize'], len(wad))
    seq_read(log, fileopen.K['ReadFile'], WAD, len(wad), buffer=1, count=2, done=3, returns=False)
    seq_seek(log, fileopen.K['SetFilePointer'], offset=1, origin=3)
    regions[WAD] = wad + bytes(4)
    regions[FOLDERTEXT] = folder + b'\0' + r.bytes(8)
    if name is not None:
        regions[NAME_TEXT] = name + b'\0' + r.bytes(8)
    regions[SOUND_SYSTEM] = struct.pack('<I', SOUND)
    sound = bytearray(r.bytes(0x44))
    sound[0x30:0x44] = string(r, root, SOUNDTEXT)
    regions[SOUND] = sound
    regions[SOUNDTEXT] = root + b'\0' + r.bytes(8)
    regions[OUT] = r.bytes(8)
    if obj:   # (0x6b8720 on the singleton: the folder split into the object's channel's base directory (+0x30) and a relative path, or the
        # whole folder as the (absolute) path; no channel now and then: only *out)
        regions[SINGLETON] = regions.pop(CAT)
        regions[0xfb1ff8] = bytes([r.choice([1, r.getrandbits(8) | 1])])
        k = r.choice([0, 3, 6, len(folder)])
        base, path = (folder[:k], folder[k:]) if k else (r.choice([b'', b'E:\\x\\']), folder)
        chan = bytearray(r.bytes(0x44))
        chan[0x30:0x44] = string(r, base, OBJ_BASE)
        regions[OBJ_CHAN], regions[OBJ_BASE], regions[OBJ_PATH] = chan, base + b'\0' + r.bytes(8), path + b'\0' + r.bytes(4)
        regions[OBJ] = r.bytes(0x24) + struct.pack('<I', OBJ_CHAN if r.random() < 0.95 else 0) + r.bytes(8)
        return Call(CALLS['objload'], [OBJ, OUT, OBJ_PATH, NAME_TEXT if name is not None else 0, OUT + 4], regions, conv='t', ret='a')
    return Call(CALLS['load'], [CAT, OUT, FOLDERTEXT, NAME_TEXT if name is not None else 0, OUT + 4], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
