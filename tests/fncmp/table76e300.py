# The counted list erase, person match, setting switch, slot push, table lookups and list clear of src/game/core/small12 against the
# originals: any object bytes and arguments, the table of 56-byte records at 0x76e300 filled with ids at their own slot and elsewhere.
# usage: small12.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Name,...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, HEAP, RAM, heap, put  # noqa: E402

CALLS = {'Node': (0x4f54e0, 't', 'a'), 'Has': (0x5065d0, 't', 'a'), 'Mode': (0x550d80, 't', 'v'), 'Quad': (0x5633e0, 's', 'a'),
         'GetD': (0x59c390, 't', 'a'), 'Set': (0x59c410, 't', 'b'), 'Clear': (0x5b15a0, 't', 'v'), 'Read': (0x550ca0, 't', 'a'), 'Zero': (0x4c9050, 'c', 'b')}
B = Bench([(0x4f54e0, 0x4f5560), (0x5065d0, 0x506650), (0x550d80, 0x550e00), (0x5633e0, 0x563460), (0x59c390, 0x59c410), (0x59c410, 0x59c490),
           (0x5b15a0, 0x5b1620), (0x4f23f0, 0x4f2440), (0x550ca0, 0x550d80), (0x4c9050, 0x4c9070), (0x51a280, 0x51a290), (0x5da3c0, 0x5da3c1)])
LIST, BUFS, TABLE = RAM + 0x1000, RAM + 0x2000, 0x76e300
NREC = 342


def records(r):
    """The 56-byte records: the id at +4 (mostly the slot's own number), a small index at +8."""
    data = bytearray(r.bytes(56 * NREC + 0x40))
    for i in range(NREC):
        put(data, 56 * i + 4, 'I', i if r.random() < 0.7 else r.choice([0, 1, 5, 17, r.randrange(NREC)]))
        put(data, 56 * i + 8, 'I', r.randrange(4))
    return data


def case(r):
    name = r.choice(OPS)
    address, conv, ret = CALLS[name]
    obj = bytearray(r.bytes(0x800))
    regions = {RAM: obj, HEAP: heap()}
    if name == 'Node' or name == 'Clear':
        n = r.choice([0, 1, 2, 3, 4])
        nodes = [LIST + 0x30 * (i + 1) for i in range(n)]
        order = [LIST] + nodes
        data = bytearray(r.bytes(0x30 * (n + 1)))
        bufs = bytearray(r.bytes(0x200))
        for i, a in enumerate(order):
            put(data, a - LIST, 'II', order[(i + 1) % len(order)], order[i - 1])
            use = r.choice([0, 1, 2, 5, 0xfe, 0xff])
            bufs[0x30 * i + 2] = use
            # the counted buffers of the nodes (at +0xc for Node; Clear's nodes hold them at +8 + 4)
            put(data, a - LIST + 0xc, 'I', r.choice([0, BUFS + 0x30 * i + 3, BUFS + 0x30 * i + 3]))
        regions.update({LIST: data, BUFS: bufs})
        if name == 'Clear':
            put(obj, 4, 'I', LIST)
            put(obj, 8, 'I', r.choice([n, n + 1, 0, 100]))
            return Call(address, [RAM], regions, conv='t', ret=ret)
        put(obj, 8, 'I', r.choice([n, n + 1, 0, 100]))
        return Call(address, [RAM, RAM + 0x300, r.choice(order)], regions, conv='t', ret=ret)
    if name == 'Has':
        obj[2] = r.choice([0, 3, 4, 4, 5, 5, 6])
        w = r.choice([0, 1, 0x1234, 0xffff])
        put(obj, 0x218, 'HH', w if r.random() < 0.6 else r.getrandbits(16), w if r.random() < 0.6 else r.getrandbits(16))
        put(obj, 0x19c, 'I', r.choice([0xb, 0xd, 0xb, 0xd, 0, 5]))
        return Call(address, [RAM, w | r.choice([0, 0x10000])], regions, conv='t', ret=ret)
    if name == 'Read':
        regions[0x7a1a8c] = struct.pack('<I', r.choice([0, 0x800000, 0x1000000, 0x2000000, 0x3800000, 0xffffffff, r.getrandbits(32)]))
        return Call(address, [RAM], regions, conv='t', ret=ret)
    if name == 'Zero':
        game = RAM + 0x40000
        regions.update({0x80239c: struct.pack('<I', game - 0x1da710), game: struct.pack('<I', r.choice([0, 0, 1, 0x100, 0xffffffff]))})
        return Call(address, [], regions, conv='c', ret=ret)
    if name == 'Mode':
        regions[0x7a1a8c] = r.bytes(4)
        return Call(address, [RAM, r.choice([0, 1, 2, 3, 4, 0xffffffff, r.getrandbits(32)])], regions, conv='t', ret=ret)
    if name == 'Quad':
        src = RAM + 0x400
        put(obj, 0x788, 'i', r.choice([0, 1, 2, 3, 4, 5, -1, 3]))
        return Call(address, [src if r.random() < 0.8 else 0, r.getrandbits(32), r.getrandbits(32), RAM], regions, conv='s', ret=ret)
    regions[TABLE] = records(r)
    return Call(address, [RAM, r.choice([0, 1, 2, 5, 17, 100, 0x15e, 0x15f, 0x160, 0x161, 200, 300, 341])], regions, conv='t', ret=ret)


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
