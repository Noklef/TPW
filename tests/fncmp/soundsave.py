# The sound system's save / load chain against the originals, on one fixture for the saved records.
# The names' map (src/game/core/soundnames: 0x6c10f0, 0x6c0d90, 0x6c1050, 0x6c0f50): the singleton category at 0xfb1f28 (soundcategory.py's,
# its guard set) with deque A's records named as soundcategory.py's findrec case; its map at +4 a count and a block (0 now and then). The load
# reads from a memory stream at STREAM over DATA: a size (small, now and then short of the names or past the stream), the count (0-5, the
# names' count or not) and the block: the count's dwords, then each name a length byte (its text's length + 1, 0, or the length alone) and
# its text (one of the records' names or another). The block comes from the heap (failing now and then: a fault in both).
# usage: soundsave.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import (HEAP, IMPORT_TABLE, LOG, RAM, Bench, Call, blob_bytes, heap, import_table, log_region, memory_stream, recorder,  # noqa: E402
                   set_return, strings, vtable)

CALLS = {'mapfree': 0x6c10f0, 'freemap': 0x6c0d90, 'mapload': 0x6c1050, 'namesload': 0x6c0f50, 'vsave': 0x6bccf0, 'vload': 0x6bd0a0,
         'msave': 0x6b6c30, 'mload': 0x6b6da0, 'osave': 0x6ba0c0, 'oload': 0x6ba150, 'csave': 0x6b5c10, 'cload': 0x6b5c30, 'gsave': 0x51c350,
         'gload': 0x51c400}


def merged(ranges):
    out = []
    for lo, hi in sorted(ranges):
        if out and lo <= out[-1][1]:
            out[-1] = (out[-1][0], max(out[-1][1], hi))
        else:
            out.append((lo, hi))
    return out


B = Bench(merged([(0x5f5e20, 0x5f5e90), (0x6bb220, 0x6bb250), (0x6bb950, 0x6bb9a0), (0x6bccf0, 0x6bd600), (0x6bd7a0, 0x6bd7c0), (0x6c0da0, 0x6c0dd0),
                  (0x6c3a00, 0x6c3a90), (0x6c3de0, 0x6c3e10), (0x6d1bd0, 0x6d1e40), (0x6b7150, 0x6b7190)] + [(0x47db00, 0x47db30), (0x5ec210, 0x5ec230), (0x5f61a0, 0x5f61f0), (0x5f7980, 0x5f79f0), (0x5f8240, 0x5f8260), (0x632310, 0x632360),
           (0x6338b0, 0x6338d0), (0x67a740, 0x67a800), (0x67ad90, 0x67ae10), (0x67c250, 0x67c2a0), (0x67dcd0, 0x67dd50), (0x67efc0, 0x67f1b0),
           (0x67fa20, 0x67fa50), (0x681740, 0x681970), (0x682720, 0x6827d0), (0x682b40, 0x682e50), (0x682fc0, 0x6830a0), (0x688710, 0x688740),
           (0x6bead0, 0x6bed90), (0x6c0d90, 0x6c1120), (0x6c1240, 0x6c1280), (0x6c15f0, 0x6c1aa0), (0x6c2140, 0x6c2200), (0x6c2860, 0x6c29a0),
           (0x6c2ca0, 0x6c2d30), (0x6c31a0, 0x6c31f0)]
          + [(0x5f5f10, 0x5f5fc0), (0x6b6440, 0x6b64c0), (0x6b6930, 0x6b6b50), (0x6b6c30, 0x6b7050), (0x6b7150, 0x6b71a0), (0x6b71d0, 0x6b7200)]
          + [(0x6ba0c0, 0x6ba200), (0x6b5c10, 0x6b5c60), (0x51c350, 0x51c580), (0x6b5bd0, 0x6b5c00), (0x6b8b40, 0x6b8b70), (0x6b5c90, 0x6b5cc0),
             (0x5f5bf0, 0x5f5c20), (0x6b9260, 0x6b9280), (0x6b9c50, 0x6b9c80), (0x6b9cd0, 0x6b9d00), (0x6c6000, 0x6c60a0)]))
SINGLETON, GUARD = 0xfb1f28, 0xfb1ff8
OUT, STREAM, DATA, MAPAT, BLOCK = RAM + 0xa000, RAM + 0xa100, RAM + 0xa200, RAM + 0xa800, RAM + 0xa900
OPS = [n for n in CALLS if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]


def singleton(r):
    """The singleton category (its guard set) with deque A's records named (soundcategory.py's findrec case); returns (regions, names)."""
    import soundcategory as sc
    regions = sc.category(r, 4, holes=True, many=r.random() < 0.1)
    names = [r.choice(sc.NAMES_GIVEN + [b'Bank0', b'ab']) for _ in range(4)]
    recs = bytearray(regions[sc.RECS])
    for i in range(len(recs) // 0x40):
        struct.pack_into('<I', recs, 0x40 * i + 8, r.choice([0, 1, 1, r.getrandbits(32)]))
        recs[0x40 * i + 0xc:0x40 * i + 0x20] = sc.string(r, names[i], sc.NAMES + 0x40 * i)
        regions[sc.NAMES + 0x40 * i] = names[i] + b'\0' + r.bytes(8)
    regions[sc.RECS] = recs
    regions[SINGLETON] = regions.pop(sc.CAT)
    regions[GUARD] = bytes([r.choice([1, r.getrandbits(8) | 1])])
    return regions, names


def names_block(r, names):
    """A saved names' block: (count, the bytes after the size and count)."""
    count = r.choice([0, 1, 2, 3, 5])
    body = bytearray(r.bytes(4 * count))
    for _ in range(count):
        text = r.choice(names + [b'', b'Bank', b'x', b'Bank0.bnk'])
        n = r.choice([len(text) + 1] * 4 + [0, len(text)])
        body += bytes([n]) + (text + b'\0')[:n] + r.bytes(max(0, n - len(text) - 1))
    return count, bytes(body)


def names_case(r, name):
    regions, names = singleton(r)
    regions[HEAP] = heap(0x800, fail=r.choice([0] * 12 + [1]))
    cat = bytearray(regions[SINGLETON])
    struct.pack_into('<II', cat, 4, r.getrandbits(32), r.choice([0, BLOCK]))
    regions[SINGLETON] = cat
    regions[BLOCK] = r.bytes(0x10)
    regions[OUT] = r.bytes(4)
    if name == 'mapfree':
        regions[MAPAT] = struct.pack('<II', r.getrandbits(32), r.choice([0, BLOCK]))
        return Call(CALLS[name], [r.choice([MAPAT, SINGLETON + 4])], regions, conv='t', ret='v')
    if name == 'freemap':
        return Call(CALLS[name], [SINGLETON], regions, conv='t', ret='v')
    count, body = names_block(r, names)
    if r.random() < 0.15:
        count = r.choice([count + 1, max(0, count - 1), r.randrange(8)])
    size = r.choice([len(body)] * 6 + [max(0, len(body) - r.randrange(1, 8)), len(body) + r.randrange(1, 0x10)])
    data = struct.pack('<II', size, count) + body + r.bytes(0x20)
    left = r.choice([len(data)] * 8 + [r.randrange(len(data))])
    regions[STREAM] = memory_stream(STREAM, DATA, left)
    regions[DATA] = data
    if name == 'mapload':
        regions[MAPAT] = struct.pack('<II', r.getrandbits(32), r.choice([0, BLOCK]))
        return Call(CALLS[name], [r.choice([MAPAT, SINGLETON + 4]), OUT, STREAM], regions, conv='t', ret='a')
    return Call(CALLS[name], [SINGLETON, OUT, STREAM], regions, conv='t', ret='a')


MAGIC = 0x12d591
VOICE, VVT, PARAMS, LINKED, LPARAMS, BUF = RAM + 0xb000, RAM + 0xb100, RAM + 0xb200, RAM + 0xb300, RAM + 0xb400, RAM + 0xc000
TYPE_K = 20   # the voices' type (slot 20)


def voice_record(r, linked=True, bad=0.05):
    """A voice's saved record from its magic on (as SoundVoice_Save writes it after the type): magics bad now and then; the category
    (the names' map index) mostly 0 or past the map's count; with `linked`, a linked type now and then (7 or 9; another now and then) and its
    type dword and record (with no linked one itself)."""
    def magic():
        return MAGIC if r.random() > bad else r.choice([0, MAGIC + 1, r.getrandbits(32)])
    out = struct.pack('<I', magic()) + r.bytes(12) + struct.pack('<I', r.choice([0, 0, r.getrandbits(32)])) + r.bytes(4)
    out += struct.pack('<I', r.choice([0] * 6 + [1, 2, 7, 0x1000, r.getrandbits(32)])) + r.bytes(12) + struct.pack('<I', magic())
    flag = r.choice([0, 1, 1, r.getrandbits(32)])
    out += struct.pack('<I', flag) + (r.bytes(24) if flag else b'') + struct.pack('<I', magic())
    out += bytes([r.choice([0, 1, r.getrandbits(8)])]) + r.bytes(4)
    lt = r.choice([7, 9, 7, 9, 3]) if linked and r.random() < 0.4 else 0
    out += struct.pack('<I', lt)
    if lt:
        out += r.bytes(4) + voice_record(r, False, bad)
    return out


def voice(r, at, vt, params):
    v = bytearray(r.bytes(0x50))
    struct.pack_into('<I', v, 0, vt)
    struct.pack_into('<I', v, 0x4c, params)
    return v


def voice_case(r, name):
    log = log_region()
    regions = {LOG: log, VVT: vtable({TYPE_K: 0}, 24), HEAP: heap(0x800, fail=r.choice([0] * 15 + [1, 2, 3])), OUT: r.bytes(4)}
    if name == 'vsave':   # (the singleton and the sample's place as soundcategory.py's locate case; the stream room for all or less)
        import soundcategory as sc
        loc = sc.locate_case(r, 'locate').regions
        info = loc.pop(sc.INFO)
        loc.pop(sc.OUT)
        regions.update(loc)
        regions[HEAP] = heap(0x800)
        if r.random() < 0.4:
            set_return(log, TYPE_K, r.choice([0, 1, 4, 7, 9, r.getrandbits(32)]))
        v = voice(r, VOICE, VVT, r.choice([0, PARAMS, PARAMS]))
        v[4:0x10] = info[4:0x10]
        lp = r.choice([0, LPARAMS])
        params = bytearray(r.bytes(0x20))
        struct.pack_into('<I', params, 0x18, r.choice([0, LINKED, LINKED]))
        lparams = bytearray(r.bytes(0x20))
        struct.pack_into('<I', lparams, 0x18, 0)
        lv = voice(r, LINKED, VVT, lp)
        lv[4:0x10] = info[4:0x10] if r.random() < 0.7 else r.bytes(12)
        size = 0x200
        left = r.choice([size] * 5 + [r.randrange(0, 0x60)])
        regions.update({VOICE: v, PARAMS: params, LINKED: lv, LPARAMS: lparams, STREAM: memory_stream(STREAM, BUF, left), BUF: r.bytes(size)})
        return Call(CALLS[name], [VOICE, OUT, STREAM, r.getrandbits(32)], regions, conv='t', ret='a')
    # vload: the singleton's names' map (a count 0-3 over a block of indices, 0 mostly): the categories read mostly 0 or past it
    regions[GUARD] = bytes([1])
    cat = bytearray(r.bytes(0xd0))
    count = r.choice([0, 1, 2, 3])
    struct.pack_into('<II', cat, 4, count, r.choice([BLOCK, BLOCK, 0]))
    regions[SINGLETON] = cat
    regions[BLOCK] = struct.pack('<4I', *[r.choice([0, 0, 0, r.randrange(1, 4)]) for _ in range(4)])
    if r.random() < 0.7:
        set_return(log, TYPE_K, r.choice([4, 6, 1, 7, r.getrandbits(32)]))
    params = bytearray(r.bytes(0x20))
    struct.pack_into('<I', params, 0x1c, r.choice([VOICE, LINKED, r.getrandbits(32)]))
    data = voice_record(r) + r.bytes(0x10)
    left = len(data)   # (a short stream leaves the original's locals unset: its stack, not comparable)
    regions.update({VOICE: voice(r, VOICE, VVT, r.choice([0, PARAMS])), PARAMS: params, STREAM: memory_stream(STREAM, BUF, left), BUF: data})
    return Call(CALLS[name], [VOICE, OUT, STREAM, r.getrandbits(32)], regions, conv='t', ret='a')


MIX, PAGE, NAMETEXT, TELL_K = RAM + 0xb500, RAM + 0xe000, RAM + 0xb600, 25
SLOT_K, RETIRE_K = 6, 3   # the voices' slot 6 (out, functor) and slot 3 (SoundMixer_Retire's)


def located(r):
    """soundcategory.py's locate fixture (the singleton, the sample's place) with deque A's records named for SoundNames_Save: (regions,
    the info's sound / variation / sample dwords)."""
    import soundcategory as sc
    loc = sc.locate_case(r, 'locate').regions
    info = loc.pop(sc.INFO)
    loc.pop(sc.OUT)
    recs = bytearray(loc[sc.RECS])
    for i in range(len(recs) // 0x40):
        recs[0x40 * i + 0xc:0x40 * i + 0x20] = sc.string(r, b'x' * r.choice([0, 1, 5, 9]), NAMETEXT + 0x10 * i)
        recs[0x40 * i + 0x10] = r.choice([recs[0x40 * i + 0x10], 0xff, r.randrange(12)])
    loc[sc.RECS] = recs
    loc[NAMETEXT] = r.bytes(0x60)
    return loc, info[4:0x10]


def mixer_case(r, name):
    """The mixer (soundmixer.py's layout) with 0-4 voices of one page at PAGE (recorder vtables) in its active and free chains. msave: the
    active ones' records (as vsave's, with linked voices now and then) after the names (located()), the stream's position (slot 6) a
    recorder answering small positions. mload: the active ones retired, then a stream of the size, count, the names (none of them named
    mostly: the categories unnamed), the pages' count (the mixer's or one more: a page from the heap) and records of ids (a free voice's
    low 16 bits mostly) and types (1-6 mostly) with voice records (no linked ones: the game's vtables' slot 20 is not the driver's)."""
    import soundmixer as sm
    log = log_region()
    iat = import_table(log, sm.IMPORTS)
    blob_bytes(log, SLOT_K, 1, 0xff)
    strings(log, SLOT_K, 0, out=1)
    n = r.choice([0, 1, 2, 3, 4])
    page = bytearray(r.bytes(0xc + 0x50 * 5))
    struct.pack_into('<I', page, 8, 0)
    for i in range(n):
        e = 0xc + 0x50 * i
        struct.pack_into('<I', page, e, VVT)
        struct.pack_into('<I', page, e + 0x2c, r.choice([i, i | r.getrandbits(16) << 16, i + 7]))
        struct.pack_into('<I', page, e + 0x30, r.getrandbits(32) & ~0x410 | r.choice([0, 0x10, 0x400, 0x410, 0x410]))
        struct.pack_into('<I', page, e + 0x4c, r.choice([0, PARAMS]))
    order = list(range(n))
    r.shuffle(order)
    k = r.randrange(0, n + 1)
    mixer = bytearray(r.bytes(0x54))
    for at, ids in ((0x24, order[:k]), (0x20, order[k:])):
        addrs = [PAGE + 0xc + 0x50 * i for i in ids]
        struct.pack_into('<I', mixer, at, addrs[0] if addrs else 0)
        for j, a in enumerate(addrs):
            struct.pack_into('<II', page, a - PAGE + 0x34, addrs[j + 1] if j + 1 < len(addrs) else 0, addrs[j - 1] if j else 0)
    struct.pack_into('<III', mixer, 0x1c, PAGE, 0, 0)
    struct.pack_into('<II', mixer, 0x28, r.choice([1, 1, 2]), r.randrange(0x100))
    params = bytearray(r.bytes(0x20))
    struct.pack_into('<I', params, 0x18, 0)
    regions = {LOG: log, IMPORT_TABLE: iat, sm.CLOCK: bytes(0x20), VVT: vtable({RETIRE_K: 0, SLOT_K: 2, TYPE_K: 0}, 24), OUT: r.bytes(4), MIX: mixer,
               PARAMS: params}
    if name.endswith('save'):
        loc, info = located(r)
        regions.update(loc)
        regions[HEAP] = heap(0x800)
        for i in range(n):
            page[0xc + 0x50 * i + 4:0xc + 0x50 * i + 0x10] = info if r.random() < 0.8 else r.bytes(12)
        if r.random() < 0.3:
            struct.pack_into('<I', params, 0x18, LINKED)
            regions.update({LINKED: voice(r, LINKED, VVT, 0)})
            regions[LINKED][4:0x10] = info
        if r.random() < 0.5:
            set_return(log, TYPE_K, r.choice([0, 1, 4, 7, r.getrandbits(32)]))
        stream = memory_stream(STREAM, BUF, r.choice([0x400] * 5 + [r.randrange(0x100)]))
        struct.pack_into('<I', stream, 0x20 + 0x18, recorder(TELL_K, 0))
        set_return(log, TELL_K, r.choice([0, 0, 4, 8, 0x40, r.randrange(0x400), 0x401]))
        regions.update({PAGE: page, STREAM: stream, BUF: r.bytes(0x400)})
        return wrap(r, name, regions, None, 0)
    sing, names = singleton(r)
    regions.update(sing)
    regions[HEAP] = heap(0x2800, fail=r.choice([0] * 15 + [1, 2]))
    restore = r.choice([1, 1, 1, 1, 0, 0x100, r.getrandbits(32)])
    count, body = names_block(r, [b'zz', b'qq'] if r.random() < 0.8 else names)
    pages = struct.unpack_from('<I', mixer, 0x28)[0] + r.choice([0, 0, 0, 1, -1])
    ids = [struct.unpack_from('<I', page, 0xc + 0x50 * i + 0x2c)[0] for i in order[k:]]
    recs = b''
    m = r.choice([0, 1, 2, 3, 4])
    for _ in range(m):
        vid = r.choice(ids * 3 + [r.getrandbits(32), 0, 1]) if ids else r.choice([0, 1, r.getrandbits(32)])
        recs += struct.pack('<II', vid, r.choice([1, 2, 3, 4, 5, 6] * 2 + [0, 7, r.getrandbits(32)])) + voice_record(r, False, 0.02)
    data = struct.pack('<I', r.getrandbits(8)) + struct.pack('<I', r.choice([m, m, m, r.randrange(5)]))
    data += struct.pack('<II', len(body), count) + body + struct.pack('<I', pages & 0xffffffff) + recs + r.bytes(0x200)
    regions[PAGE] = page
    return wrap(r, name, regions, data, restore)


OBJ, CMD, MGRH, CHAN = RAM + 0xb700, RAM + 0xb780, RAM + 0xb7c0, RAM + 0xb800
GLOBALS, SETTINGS = 0x802bc0, 0x803aa0   # (on 0x802bc8, the manager 0x802bcc, started 0x802bd4; the 0x54 bytes, the EAX record +0x14)


def wrap(r, name, regions, data, restore):
    """The mixer's call (m*) or the callers' over it: the sound object (o*: its mixer +0x20; the load's three dropped dwords first), its
    commands (c*: 0x700bb0 / 0x700bb4 with the stream and the restore byte) and the globals' save / load (g*: the manager's channel the sound
    object; the load's on flag, size (0x54 mostly) and bytes first, short now and then before the mixer's part; its EAX record apply with
    the channel's flags not both bits 0 and 1: *out = -1). data: the load's mixer part (None: a save)."""
    kind = name[0]
    obj = bytearray(r.bytes(0x40))
    struct.pack_into('<II', obj, 0x20, MIX, CHAN)
    chan = bytearray(r.bytes(0x40))
    chan[0x28] = chan[0x28] & ~3 | r.choice([0, 1, 2])
    regions.update({OBJ: obj, CHAN: chan, CMD: struct.pack('<II', r.getrandbits(32), STREAM) + bytes([restore & 0xff]) + r.bytes(7)})
    if data is not None:
        left = None
        if kind in 'ocg':
            data = r.bytes(12) + data
        if kind == 'g':
            size = r.choice([0x54] * 6 + [0, 0x10, 0x58])
            head = struct.pack('<II', r.getrandbits(32), r.choice([0, 1, 1, r.getrandbits(32)])) + struct.pack('<I', size) + r.bytes(size)
            data = head + data
            if r.random() < 0.1:
                left = r.randrange(len(head))
        regions.update({STREAM: memory_stream(STREAM, BUF, len(data) if left is None else left), BUF: data})
    if kind == 'm':
        return Call(CALLS[name], [MIX, OUT, STREAM] + ([restore] if data is not None else []), regions, conv='t', ret='a')
    if kind == 'o':
        return Call(CALLS[name], [OBJ, OUT, STREAM] + ([restore] if data is not None else []), regions, conv='t', ret='a')
    if kind == 'c':
        return Call(CALLS[name], [CMD, OUT, OBJ], regions, conv='t', ret='a')
    g = bytearray(r.bytes(0x20))
    struct.pack_into('<II', g, 8, r.choice([0, 1, 1, 1, r.getrandbits(32)]), r.choice([MGRH, MGRH, 0]) if data is not None else MGRH)
    struct.pack_into('<I', g, 0x14, r.choice([0, 1, 1, 1]))
    regions.update({GLOBALS: g, SETTINGS: r.bytes(0x60), MGRH: struct.pack('<II', r.getrandbits(32), OBJ)})
    return Call(CALLS[name], [STREAM], regions, conv='c', ret='v')


def case(r):
    name = r.deal(OPS)
    if name in ('vsave', 'vload'):
        return voice_case(r, name)
    if name in ('mapfree', 'freemap', 'mapload', 'namesload'):   # (before the kinds: 'mapfree' / 'mapload' start with the mixer's 'm')
        return names_case(r, name)
    return mixer_case(r, name)


if __name__ == '__main__':
    B.main(case)
