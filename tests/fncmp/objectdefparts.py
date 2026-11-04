# A definition's track types read (src/game/world/objectdefs: 0x414420) and a coaster type read for one (src/game/coaster/setup: 0x42fad0) against the originals. 0x414420: the handle's find and next recorders writing results and names
# through the record (names ending in the suffix or not, the last character a digit or not), the open failing now and then. 0x42fad0: the create recorder answering a type: pieces the same or not, section counts about 0, 1, 2, optional parts or not, floats and NaNs.
# usage: objectdefparts.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=TrackTypes,TypeLoad)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from fncmp import HEAP, LOG, RAM, Bench, Call, _recorder, blob, echo, fail_mask, fill, heap, log_region, put, set_return, strings, vtable  # noqa: E402

CD = {0x4629d0: (12, 11), 0x51fa20: (31, 1), 0x42fd90: (1, 4), 0x424ef0: (2, 4), 0x424ed0: (3, 1), 0x46f150: (4, 1), 0x430130: (5, 2), 0x430590: (6, 3)}
B = Bench([(0x414420, 0x4145c5), (0x42fad0, 0x42fd84), (0x424d30, 0x424db0), (0x424cf0, 0x424d10), (0x5f5fc0, 0x5f7f00), (0x67a400, 0x6902c0)], hooks={a: _recorder(k, n, cdecl=True) for a, (k, n) in CD.items()})
SOURCE, VT, DEF, TEXTS = RAM + 0x1200, RAM + 0x1300, RAM + 0x2000, RAM + 0x3000
TYPE, TYPES, HOLDER, ANIM, RECORD, PIECES = RAM + 0x4000, RAM + 0x4800, RAM + 0x4900, RAM + 0x4980, RAM + 0x49c0, RAM + 0x4a00
STRING_VT = 0x703090
OPS = os.environ.get('OPS', 'TrackTypes,TypeLoad').split(',')


class Texts:
    """C strings in one region."""
    def __init__(self):
        self.buf = bytearray()

    def add(self, text):
        at = TEXTS + len(self.buf)
        self.buf += text + b'\0'
        return at


def string(texts, text):
    return struct.pack('<5I', STRING_VT, len(text), texts.add(text), (len(text) & ~7) + 8, 8)


def track_types(r):
    log = log_region()
    texts = Texts()
    suffix = b'.emt'
    def name():
        return r.choice([b'T%d.emt' % r.randrange(10), b'T%d.EMT' % r.randrange(10), b'T%d.xyz' % r.randrange(10), b'Tx.emt', b'.emt', b'x', b'']).ljust(8, b'\0')
    n = r.choice([0, 1, 2, 3, 5])
    def res(v):
        return struct.pack('<i', v)
    good = [0, 1, 0x7fffffff]
    bad = [-1, -2, -0x80000000]
    fill(log, 1, 0, res(r.choice(good * 5 + bad)))
    fill(log, 1, 2, name(), which=1, offset=0x24)
    echo(log, 1, 0)
    strings(log, 1, 2, 5)
    results = b''.join(res(r.choice(good)) for _ in range(n)) + res(r.choice(bad))
    fill(log, 2, 0, results, which=2, records=n + 1)
    fill(log, 2, 1, b''.join(name() for _ in range(n + 1)), which=3, offset=0x24, records=n + 1)
    echo(log, 2, 0)
    strings(log, 2, 0, 3)
    fill(log, 3, 0, b''.join(res(r.choice(good * 4 + bad)) for _ in range(n + 1)), which=4, records=n + 1)
    echo(log, 3, 0)
    strings(log, 3, 4, 3)
    strings(log, 31, 0, 1)
    set_return(log, 31, r.getrandbits(32))
    regions = {LOG: log, VT: vtable({1: 3, 2: 2, 3: 5}), DEF: r.bytes(0x510), SOURCE: struct.pack('<I', VT), HEAP: heap(0x2000)}
    regions[0x789010] = string(texts, suffix)
    regions[0x788f40] = struct.pack('<I', texts.add(b'*.emt'))
    regions[TEXTS] = texts.buf
    return Call(0x414420, [DEF, SOURCE], regions, conv='t', ret='v')


def type_load(r):
    log = log_region()
    texts = Texts()
    folder = texts.add(r.choice([b'Data\\Rides\\Big', b'R', b'']))
    t = bytearray(r.bytes(0x400))
    put(t, 0x14, 'I', HOLDER)
    put(t, 0x18, 'I', ANIM)
    put(t, 0x1c, 'I', RECORD)
    p = [PIECES + 0x40 * i for i in range(3)]
    middle = p[0]
    put(t, 0x300, 'I', middle)
    put(t, 0x2fc, 'I', r.choice([middle, p[1]]))
    put(t, 0x304, 'I', r.choice([middle, p[2]]))
    put(t, 0x2f4, 'I', r.choice([0, 1, 2, 3, 7, 40]))
    put(t, 0xb8, 'I', r.choice([0, texts.add(b'PartA')]))
    put(t, 0x1d8, 'I', r.choice([0, texts.add(b'PartB')]))
    pieces = bytearray()
    for _ in p:
        piece = bytearray(r.bytes(0x40))
        for off in (0x24, 0x28):
            put(piece, off, 'I', r.choice([0x7fc00001, 0xffc00002, 0x7f800001, 0x7f800000]) if r.randrange(12) == 0 else struct.unpack('<I', struct.pack('<f', r.f(1000.0)))[0])
        pieces += piece
    set_return(log, 1, TYPE)
    strings(log, 1, 6)
    set_return(log, 2, r.choice([0, 1]))
    strings(log, 2, 4)
    strings(log, 5, 2)
    strings(log, 12, 3)
    blob(log, 12, 2, 3)   # (the stack's cells: their first two dwords set)
    fail_mask(log, 12, r.choice([3, 3, 3, 0, 1, 2]))
    slot = r.randrange(4)
    table = b''.join(struct.pack('<I', TYPES + 8 * i) for i in range(4))
    regions = {LOG: log, TYPE: t, TYPES: r.bytes(0x20), HOLDER: r.bytes(0x40), ANIM: r.bytes(0x10), RECORD: r.bytes(0x10), PIECES: pieces, 0x7ae6a8: table,
               0x790fd0: struct.pack('<i', r.choice([0, r.randrange(-100, 100), r.getrandbits(31)])), 0x791cfc: struct.pack('<I', r.choice([0, r.randrange(0x10000), r.getrandbits(32)]))}
    regions[TEXTS] = texts.buf
    return Call(0x42fad0, [slot, folder, r.getrandbits(32)], regions, conv='c', ret='v')


CASES = {'TrackTypes': track_types, 'TypeLoad': type_load}


def case(r):
    return CASES[r.deal(OPS)](r)


if __name__ == '__main__':
    B.main(case)
