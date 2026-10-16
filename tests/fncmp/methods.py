# The small methods of src/game/core/methods against the originals: any object bytes with the fields they test set to the
# interesting values, status pointers, 16- and 8-byte block ranges, record tables.
# usage: methods.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Get18,...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM  # noqa: E402

# name: (address, convention, returns, size, popped bytes marker unused)
CALLS = {
    'Get18': (0x6d4b00, 't', 'a', 0x10), 'Mark': (0x6d4b10, 't', 'a', 0x40), 'Bit1': (0x6c5ce0, 't', 'a', 0x40),
    'Rate': (0x6c6f90, 't', 'v', 0x40), 'Word': (0x6b00a0, 't', 'v', 0x40), 'Clear': (0x6bbe40, 't', 'a', 0x40),
    'Pick': (0x6d6c70, 't', 'a', 0x30), 'SetRate': (0x6d6cb0, 't', 'a', 0x40), 'Get20': (0x6d6db0, 's', 'a', 0x20),
    'Copy16': (0x6af6f0, 'c', 'a', 0x40), 'Erase': (0x6af060, 't', 'a', 0x40), 'Kind': (0x6adf20, 't', 'a', 0x34),
    'Rec': (0x6ace60, 't', 'a', 0x40), 'Range': (0x6ad8f0, 't', 'a', 0x58), 'Mask': (0x6bc040, 't', 'b', 0x4a),
    'Remap': (0x6d22d0, 't', 'a', 0x48), 'RangeW': (0x6ad900, 't', 'a', 0x48), 'Adv': (0x6b9b30, 't', 'v', 0x68)}
B = Bench([(a, a + n) for (a, _c, _r, n) in CALLS.values()])
VALUES = [0, 1, 2, 3, 0x10, 0x40, 0x4000, 0x4800, 0x800, 0xff, 0x100, 0x7fffffff, 0x80000000, 0xffffffff, 0xfffffffe]
POOL = RAM + 0x3000
OUT = RAM + 0x1000


def case(r):
    name = r.choice(OPS)
    address, conv, ret, _n = CALLS[name]
    obj = bytearray(r.bytes(0x100))
    for off in range(0, 0x60, 4):
        if r.random() < 0.5:
            struct.pack_into('<I', obj, off, r.choice(VALUES))
    pool = bytearray(r.bytes(0x400))
    out = struct.pack('<I', r.choice(VALUES))
    regions = {RAM: obj, POOL: pool, OUT: out}
    args = [RAM]
    if name == 'Get18':
        struct.pack_into('<I', obj, 0x14, r.choice([0, POOL]))
    elif name == 'Mark':
        struct.pack_into('<I', obj, 0x14, r.choice([0, 1, POOL]))
        regions[RAM + 0x800] = struct.pack('<I', POOL + 0x20)
        args += [OUT, RAM + 0x800, r.choice(VALUES)]
    elif name == 'Bit1':
        args += [OUT, r.choice(VALUES)]
    elif name == 'Rate':
        obj[0x20] = r.choice([0, 0xff, 0xe3, 0x1c, r.getrandbits(8)])
        args += [r.choice([0x5622, 0xac44, 0x2b11, 0, 0x15622, 0xffff5622, 0x1ac44, r.getrandbits(8)])]
    elif name == 'Word':
        end = POOL + 0x100
        cur = r.choice([end, end, POOL + 0x10, end - 2])
        struct.pack_into('<II', obj, 4, cur, end)
        obj[0x10] = r.choice([0, 0, 1])
        args += [r.choice([0, 0x1234, 0xffff, 0x12345678])]
    elif name == 'Clear':
        key = r.choice(VALUES)
        struct.pack_into('<I', obj, 0x2c, key)
        args += [r.choice(VALUES), key if r.random() < 0.7 else r.choice(VALUES), r.choice([0, 0, 1])]
    elif name == 'Pick':
        args += [OUT, POOL + r.choice([0, 0x10])]
        struct.pack_into('<I', pool, 0x4c, r.choice(VALUES))
        struct.pack_into('<I', pool, 0x5c, r.choice(VALUES))
    elif name == 'SetRate':
        args += [OUT, r.choice([0, 1, 0xff, 0x80000000])]
        struct.pack_into('<I', obj, 0xc, r.choice([0, 11025, 22050, 11025, 22050, 5]))
    elif name == 'Get20':
        args = [RAM]
    elif name == 'Copy16':
        n = r.randrange(0, 8)
        args = [POOL, POOL + 0x10 * n, POOL + 0x200 + 0x10 * r.randrange(0, 4)]
    elif name == 'Erase':
        n = r.randrange(0, 8)
        first = r.randrange(0, n + 1)
        struct.pack_into('<I', obj, 8, POOL + 8 * n)
        args += [POOL + 8 * r.randrange(0, first + 1), POOL + 8 * first]
    elif name == 'Kind':
        args += [OUT, r.choice([0, 1, 2, 3, 4, 5, 6, 7, -1, -2, 0x7fffffff, 0x80000000]) & 0xffffffff]
    elif name == 'Rec':
        count = r.randrange(0, 5)
        struct.pack_into('<II', obj, 8, count, POOL)
        for i in range(5):  # a table of pointers to records of a head, data and a terminator
            struct.pack_into('<I', pool, 4 * i, POOL + 0x40 + 0x30 * i)
            head = r.choice([1, 1, 1, 2, 0x101, 0x201, 0x301, 0x401 + 0x100 * r.randrange(4), r.getrandbits(8)])
            struct.pack_into('<I', pool, 0x40 + 0x30 * i, head)
            for j in range(1, 8):
                if r.random() < 0.6:
                    struct.pack_into('<I', pool, 0x40 + 0x30 * i + 4 * j, 0)
        args += [r.choice([0, 1, 2, 3, 4, 5, 6, 0xffffffff])]
    elif name in ('Range', 'RangeW'):
        for at_, lo in ((0, 10), (0x40, 3), (0x80, 20)):
            hi = lo + r.randrange(0, 5)
            struct.pack_into('<HHHH', pool, at_, lo, r.choice([0, 0x40, 0x80]) if at_ < 0x80 else 0,
                             hi, r.choice([0, 0x40, 0x80]) if at_ < 0x80 else 0)
        struct.pack_into('<I', obj, 8, POOL + r.choice([0, 0, 0x40]))
        args += [r.choice([0, 3, 5, 9, 10, 11, 14, 15, 20, 22, 25, 26, 30, r.randrange(0, 40), 0x10000 + r.randrange(0, 30)])]
    elif name == 'Mask':
        struct.pack_into('<I', obj, 8, POOL)
        struct.pack_into('<I', obj, 0x4c, POOL + 0x100)
        struct.pack_into('<HH', pool, 0x18, r.choice([0, 1, 2, 0x80, 0xffff]), 0)
        struct.pack_into('<H', pool, 0x1c, r.choice([0, 1, 4, 0x8000, 0xffff]))
        args += [r.choice([0, 1, 2, 4, 0x80, 0x8000, 0x10000, 0xffff, 0xffffffff]), OUT]
    elif name == 'Remap':
        struct.pack_into('<I', obj, 0xc, r.choice([11025, 22050, 0, 11026]))
        args += [r.getrandbits(32) if r.random() < 0.3 else r.choice([0, 1, 4, 8, 0x10, 0x40, 0x1000, 0x1001, 0x105d, 0xffffffff])]
    elif name == 'Adv':
        first = RAM + 0x10000 + 0x1000 * r.randrange(0, 4)
        slot = r.randrange(4, 9)
        for i in range(16):
            struct.pack_into('<I', pool, 0x100 + 4 * i, RAM + 0x10000 + 0x1000 * i)
        first = RAM + 0x10000 + 0x1000 * slot
        struct.pack_into('<IIII', obj, 0, first, first + 0x1000, first + 4 * r.choice([0, 1, 5, 0x3ff, 0x400, r.randrange(0, 0x401)]), POOL + 0x100 + 4 * slot)
        args += [r.choice([0, 1, -1, 5, -5, 0x3ff, 0x400, -0x400, 0x401, -0x401, 2000, -2000, 2500, -2500, r.randrange(-3000, 3000)]) & 0xffffffff]
    return Call(address, args, regions, conv=conv, ret=ret)


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
