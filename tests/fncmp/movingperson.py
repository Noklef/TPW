# A moving person (src/game/world/person) against the original: its serializer (0x4f8b10) and its destination by cell (0x4fa530) or marked
# (0x4f7de0). All on navpath.py's person (objectblocks.py's object and block values, the nav's position in the destination's cell).
# The serializer: the folder (+0x24) one of a table of nonzero frame counts; saves and loads through a record stream with room for all the
# fields, none, or up to and about each field's end, the stream's fields the object's where the load would otherwise leave the box (the
# thing's cell, the destination, the folder, the nav's mode and position; the limit +0x198 mostly); versions in and out of 0..1 (the
# destination then set all the same). The cell destination: ids over the map, 0 and past it. The marked one: the object 0x3a0 bytes.
# usage: movingperson.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=4f8b10,...)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import navpath  # noqa: E402
import serializers  # noqa: E402
from fncmp import Bench, Call, RAM, record_stream  # noqa: E402

RANGES = []
for a, b in sorted([(0x4f8b10, 0x4f9435), (0x541f60, 0x541f6f), (0x4fa530, 0x4fa5e7), (0x4f7de0, 0x4f7e1b), (0x4217f0, 0x421800)]
                   + navpath.RANGES + serializers.RANGES):
    if RANGES and a <= RANGES[-1][1]:
        RANGES[-1] = (RANGES[-1][0], max(RANGES[-1][1], b))
    else:
        RANGES.append((a, b))
B = Bench(RANGES)
STREAM, SINK = RAM + 0x2000, RAM + 0x3000
FOLDERS = 0x763f90  # (count, two dwords) per folder
NAV = serializers.FIELDS[0x50d150]
SIZES = [2] * 4 + [4] * 3 + [2, 2, 2, 2, 1, 4, 2] + NAV + [4, 4, 4, 4, 2, 4, 4, 4, 4] + [4] * 0x24  # (the thing, the script reference, ..., the thoughts)
NAV_AT = 35  # the nav's fields in the stream: its mode (+0xb4) 48 in, its position (+8) 97 in
COPIES = [(0, 4, 4), (20, 0x18, 4), (29, 0x24, 4), (NAV_AT + 48, 0xd4 + 0xb4, 4), (NAV_AT + 97, 0xd4 + 8, 8)]  # (stream, object, size)
LIMIT_AT = NAV_AT + sum(NAV) + 12  # +0x198 in the stream


def serial(r):
    c = navpath.person_case(r)
    regions = dict(c.regions)
    obj = regions[RAM]
    struct.pack_into('<HH', obj, 0x18, c.args[1] & 0xffff, c.args[2] & 0xffff)
    folders = r.choice([1, 2, 16])
    struct.pack_into('<I', obj, 0x24, r.randrange(folders))
    regions[FOLDERS] = b''.join(struct.pack('<III', r.choice([1, 2, 3, 7, r.randrange(1, 300), r.getrandbits(32) | 1]), r.getrandbits(32), r.getrandbits(32))
                                for _ in range(folders))
    sink = bytearray(r.bytes(0x1400))
    for at, field, size in COPIES + ([(LIMIT_AT, 0x198, 4)] if r.random() < 0.8 else []):
        sink[at:at + size] = obj[field:field + size]
    ends = [sum(SIZES[:i + 1]) for i in range(len(SIZES))]
    total = ends[-1]
    end = r.choice(ends)
    left = r.choice([total, total, total, total, 0x1400, 0, end, end - 1, end + 1, r.randrange(total)])
    regions[STREAM] = record_stream(STREAM, SINK, left)
    regions[SINK] = sink
    save = r.choice([0, 0, 1, 1, r.getrandbits(32)])
    version = r.choice([0, 1, 0, 1, 0, 1, -1, 2, r.getrandbits(32)])
    return Call(0x4f8b10, [RAM, STREAM, save, version & 0xffffffff, r.getrandbits(32)], regions, conv='t', ret='a')


def cell(r):
    cid = r.choice([0, 1, 0x4000, 0x4001, 0x10000, r.randrange(1, 0x4001), r.randrange(1, 0x4001), r.getrandbits(32)])
    index = (cid - 1) & 0xffffffff
    xy = ((index & 0x7f) << 8 | 0x80, (index >> 7 & 0xff) << 8 | 0x80)
    c = navpath.person_case(r, xy)
    return Call(0x4fa530, [RAM, cid], c.regions, conv='t', ret='a')


def marked(r):
    c = navpath.person_case(r)
    regions = dict(c.regions)
    regions[RAM] = bytearray(regions[RAM]) + bytearray(r.bytes(0x1a0))
    return Call(0x4f7de0, c.args, regions, conv='t', ret='a')


def case(r):
    op = r.deal(OPS or [0x4f8b10, 0x4f8b10, 0x4fa530, 0x4f7de0])
    return serial(r) if op == 0x4f8b10 else cell(r) if op == 0x4fa530 else marked(r)


OPS = [int(k, 16) for k in os.environ.get('OPS', '').split(',') if k]


if __name__ == '__main__':
    B.main(case)
