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
from fncmp import HEAP, LOG, RAM, Bench, Call, heap, log_region, memory_stream, set_return, strings, vtable  # noqa: E402

CALLS = {'mapfree': 0x6c10f0, 'freemap': 0x6c0d90, 'mapload': 0x6c1050, 'namesload': 0x6c0f50, 'vsave': 0x6bccf0, 'vload': 0x6bd0a0}


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
           (0x6c2ca0, 0x6c2d30), (0x6c31a0, 0x6c31f0)]))
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


def case(r):
    name = r.deal(OPS)
    if name in ('vsave', 'vload'):
        return voice_case(r, name)
    return names_case(r, name)


if __name__ == '__main__':
    B.main(case)
