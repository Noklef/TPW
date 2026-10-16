# The small functions of src/game/core/misc against the originals: any object bytes; the streambuf
# pointers pointing into the object or a buffer (now and then at each other), any positions.
# usage: misc.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Clear0,Setg,...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

B = Bench([(0x424ed0, 0x424ee9), (0x4922d0, 0x4922e0), (0x4e1c60, 0x4e1c70), (0x4f00f0, 0x4f0110), (0x4f6820, 0x4f6850),
           (0x4f6850, 0x4f6880), (0x56a510, 0x56a540), (0x56b630, 0x56b650), (0x56b830, 0x56b850)])
CALLS = ['Clear0', 'Clear18', 'Diff', 'Virtual', 'Setg', 'Setp', 'Pixel', 'Fill', 'Set3034']
BUF = RAM + 0x1000


def case(r):
    op = r.choice(OPS)
    obj = bytearray(r.bytes(0x400))
    regions = {RAM: obj}
    if op == 'Clear0':
        return Call(0x424ed0, [RAM + 0x10], regions, ret='a')
    if op == 'Clear18':
        return Call(0x4922d0, [RAM + 0x20], regions, conv='t', ret='a')
    if op == 'Diff':
        return Call(0x4e1c60, [RAM], regions, conv='t', ret='a')
    if op == 'Virtual':
        sub = RAM + 0x100
        put(obj, 0x100 - 4, 'I', RAM + 0x300)
        put(obj, 0x304, 'I', r.choice([0x10, 0x20, 0x4, 0x8, 0x80]))
        return Call(0x4f00f0, [sub], regions, conv='t')
    if op in ('Setg', 'Setp'):
        slots = (0xc, 0x1c, 0x2c) if op == 'Setg' else (0x10, 0x20, 0x30)
        for i, off in enumerate(slots):  # the pointers go to their own words, or all to one, or the object itself
            put(obj, off, 'I', r.choice([RAM + 0x200 + 4 * i, RAM + 0x200 + 4 * i, RAM + 0x200, RAM + off]))
        a = r.choice([0, 1, 100, 0x7fffffff, r.getrandbits(32)])
        b = r.choice([0, 2, 100, 0x80000000, r.getrandbits(32)])
        c = r.choice([0, 8, 0x1000, r.getrandbits(32)])
        if op == 'Setg':
            return Call(0x4f6820, [RAM, a, b, c], regions, conv='t', ret='a')
        return Call(0x4f6850, [RAM, a, b], regions, conv='t', ret='a')
    if op == 'Pixel':
        width = r.choice([1, 4, 16, 64])
        x, y = r.randrange(width), r.randrange(8)
        regions = {RAM: obj, BUF: r.bytes(0x400), 0x879da8: struct.pack('<IIII', 0, RAM + 0x800, 0, 0),
                   0x8bcbd0: struct.pack('<II', width, 0)}
        regions[RAM + 0x800] = bytearray(obj)  # the renderer object: +8 = the buffer
        put(regions[RAM + 0x800], 8, 'I', BUF)
        return Call(0x56a510, [x, y, RAM + 0x200], regions, conv='s', ret='a')
    if op == 'Fill':
        return Call(0x56b630, [], {0x879d00: r.bytes(0x60)}, ret='a')
    return Call(0x56b830, [RAM, r.getrandbits(32), r.getrandbits(32)], regions, conv='t', ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
