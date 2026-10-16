# The sequence helpers of src/game/core/seqs against the originals: ranges of any length, values on both sides of the limits.
# usage: seqs.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Words,...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM  # noqa: E402

# name: (address, convention, returns, size)
CALLS = {'Words': (0x6a01a0, 's', 'a', 0x30), 'Sb': (0x6a7240, 's', 'a', 0x30), 'Br': (0x6ae2b0, 'c', 'a', 0x28),
         'Rng': (0x6ae410, 'c', 'b', 0x30), 'C4': (0x6aed80, 't', 'a', 0x20), 'C88': (0x6adf90, 't', 'a', 0x20),
         'Cp8': (0x6af730, 'c', 'a', 0x30), 'Path': (0x6bfe10, 's', 'a', 0x38), 'Cpy': (0x6c4010, 's', 'a', 0x30),
         'Cmp': (0x6a7030, 's', 'a', 0x50), 'Tbl': (0x6af960, 't', 'a', 0x6c), 'Wr': (0x6afc90, 't', 'v', 0xb0)}
B = Bench([(a, a + n) for (a, _c, _r, n) in CALLS.values()])
POOL = RAM + 0x3000
OUT = RAM + 0x1000


def case(r):
    name = r.choice(OPS)
    address, conv, ret, _n = CALLS[name]
    obj = bytearray(r.bytes(0x100))
    pool = bytearray(r.bytes(0x400))
    regions = {RAM: obj, POOL: pool, OUT: r.bytes(0x100)}
    n = r.choice([0, 1, 2, 3, 5, 8, 16, 30])
    args = [RAM] if conv == 't' else []
    if name in ('Words', 'Sb'):
        size = 2 if name == 'Words' else 1
        args = [POOL, POOL + n * size]
    elif name == 'Br':
        pool[0:2] = struct.pack('<H', r.choice([0x28, 0x29, 0x2a, 0x27, 0, 0xffff, 0x128, 0x129]))
        args = [POOL]
    elif name == 'Rng':
        for i in range(8):
            struct.pack_into('<H', pool, 0x10 * i, r.choice([0, 0x5b0, 0x5b1, 0x5ea, 0x5eb, 0x5ec, 0x100, 0xffff, 0x15b1, r.getrandbits(16)]))
        for i in range(8):  # the half-way values are in range too, so that a wrong stride is seen
            if r.random() < 0.5:
                struct.pack_into('<H', pool, 0x10 * i + 8, 0x5c0)
        args = [POOL, POOL + 0x10 * r.randrange(0, 9)]
    elif name == 'C4':
        struct.pack_into('<II', obj, 4, r.choice([0, POOL]), POOL + 8 * r.choice([0, 1, 5, 100]))
    elif name == 'C88':
        struct.pack_into('<II', obj, 0x88, r.choice([0, POOL]), POOL + 8 * r.choice([0, 1, 5, 100]))
        if r.random() < 0.3:
            struct.pack_into('<II', obj, 0x88, POOL + 0x40, POOL)
    elif name == 'Path':
        text = bytearray(r.choice([b'abc\\def\\ghi', b'\\abc', b'abc', b'a\\', b'\\\\\\x', b'x' * 20 + b'\\' + b'y' * 5]) + bytes(8))
        pool[0x80:0x80 + len(text)] = text
        struct.pack_into('<I', pool, 0x20, POOL + 0x80)
        args = [POOL + 0x20, r.choice([1, 2, 3, 4, 7, 8, 12, 26, 30, r.randrange(1, 30)])]
    elif name == 'Cpy':
        count = r.choice([0, 1, 2, 3, 4, 5, 7, 8, 9, 15, 16, 17, 30, 0x40])
        args = [POOL + 0x100 + r.choice([0, 1, 2, 3, 4, 8]), POOL + 0x200 + r.choice([0, 1, 2, 3]) if r.random() < 0.7 else POOL + 0x100 + r.choice([0, 1, 4, 8]), count]
    elif name == 'Cmp':
        n1, n2 = r.choice([0, 1, 2, 3, 5, 8]), r.choice([0, 1, 2, 3, 5, 8])
        pool[0:8] = bytes(r.choice([0x41, 0x42, 0x7f, 0x80, 0xff, 0x00, 0x01]) for _ in range(8))
        pool[0x100:0x108] = bytes(pool[0:8]) if r.random() < 0.6 else bytes(r.choice([0x41, 0x42, 0x7f, 0x80, 0xff, 0x00, 0x01]) for _ in range(8))
        if r.random() < 0.3:
            pool[0x100 + r.randrange(8)] = r.choice([0x41, 0x80, 0xff])
        args = [POOL, POOL + n1, POOL + 0x100, POOL + 0x100 + n2]
    elif name == 'Tbl':
        n = r.choice([0, 1, 2, 3, 4, 5, 7, 8, 9, 15, 16])
        keys = sorted(r.sample(range(0, 40), n)) if r.random() < 0.7 else sorted(r.getrandbits(32) for _ in range(n))
        for i, k in enumerate(keys):
            struct.pack_into('<III', pool, 0x20 + 12 * i, k, r.getrandbits(32), r.getrandbits(32))
        struct.pack_into('<II', obj, 8, POOL + 0x20, POOL + 0x20 + 12 * (n - 1))
        pick = r.choice(keys + [0, 1, 39, 40, 0xffffffff, 7, 100]) if keys else r.choice([0, 1, 5])
        args = [RAM, pick if r.random() < 0.7 else r.randrange(0, 45)]
    elif name == 'Wr':
        room = r.choice([0, 1, 2, 3, 5, 8])
        cursor = OUT + r.choice([0, 3, 0x10])
        struct.pack_into('<II', obj, 4, cursor, cursor + room if r.random() < 0.9 else cursor - 1)
        obj[0x10] = r.choice([0, 0, 0, 1])
        text = bytearray(r.bytes(12))
        text[r.randrange(0, 12)] = 0
        pool[0x40:0x4c] = text
        args = [RAM, POOL + 0x40, r.choice([0xffffffff, 0xffffffff, 0, 1, 2, 3, 5, 8, 11])]
    elif name == 'Cp8':
        args = [POOL, POOL + 8 * n, OUT]
        if n == 0:
            ret = 'v'
    return Call(address, args, regions, conv=conv, ret=ret)


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
