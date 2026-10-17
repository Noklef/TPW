# The thing placement, free slot read and sprite choice of src/game/core/small61 (0x4fa930, 0x51fa20, 0x4fcac0) against the originals: small58.py's things in cells (Place), a
# table of 0x140-byte slots with the first byte set or clear read through a stream whose slot at offset 8 is a recorder (Slot), and a thing with sprite folders that are empty or not
# and a world seed (Choice).
# usage: small61.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Place,Slot,Choice)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import small58  # noqa: E402
from fncmp import Bench, Call, LOG, RAM, log_region, put, vtable  # noqa: E402

B = Bench([(0x4d91f0, 0x4d93c0), (0x4fa930, 0x4fa9a0), (0x4fcac0, 0x4fcb20), (0x50b350, 0x50b370), (0x50b6a0, 0x50b790), (0x5100e0, 0x510110), (0x510160, 0x510190), (0x516330, 0x516390),
           (0x51fa20, 0x51fa90), (0x541f60, 0x541fb0), (0x5c4f60, 0x5c4f90), (0x5da3c0, 0x5da3c1)])
STREAM, VT, WORLD, THING = RAM, RAM + 0x1000, RAM + 0x2000, RAM + 0x3000


def case(r):
    name = r.choice(OPS)
    if name == 'Place':
        c = small58.setpos(r)
        c.address = 0x4fa930
        return c
    if name == 'Slot':
        full = r.random() < 0.05
        free = r.randrange(0, 6)
        size = 0x80c7c8 - 0x804370 if full else 0x140 * (free + 1) + 0x120
        table = bytearray(r.bytes(size))
        for k in range((size - 0x118) // 0x140):
            table[0x118 + 0x140 * k] = 0 if (k == free and not full) else r.choice([1, 2, 0xff])
        regions = {0x804370: table, 0x816d40: struct.pack('<I', r.choice([1, 1, 1, 0])), STREAM: struct.pack('<I', VT) + r.bytes(0x20), VT: vtable({2: 2}), LOG: log_region()}
        return Call(0x51fa20, [STREAM], regions, conv='c', ret='a')
    counts = bytearray(0x30)
    for folder in range(4):
        put(counts, 12 * folder, 'I', r.choice([0, 1, 2, 5, 9]) if folder == 3 else r.choice([1, 2, 3, 7, 20]))
    thing = bytearray(r.bytes(0x40))
    put(thing, 0x24, 'I', r.choice([2, 2, 0, 1, 5]))
    world = RAM + 0x10000 - 0x1da708
    regions = {0x763f90: counts, THING: thing, WORLD: r.bytes(0x20), 0x80239c: struct.pack('<I', world), 0x7cf83c: struct.pack('<I', world), RAM + 0x10000: r.bytes(0x20)}
    return Call(0x4fcac0, [THING], regions, conv='t', ret='a')


OPS = [k for k in os.environ.get('OPS', 'Place,Slot,Choice').split(',') if k]

if __name__ == '__main__':
    B.main(case)
