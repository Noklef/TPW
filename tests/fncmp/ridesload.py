# The rides read back from a save (src/game/world/ridesload: 0x4647a0) against the original. The stream's front end real, its read a sequenced read; every callee a recorder (driver.cpp's kRidesLoadCalls).
# usage: ridesload.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, answers, blob, log_fold, log_region, seq_read, set_return, strings, vtable  # noqa: E402

READ = 2
ALLOC, CLOCKS, RIDE, STRIPS, HAS, SET_BLOCK, AFTER, NONE, SHOW, HIDE, FREE, FIND = 1, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13
CDECL = {0x45b9d0: (ALLOC, 2), 0x473440: (CLOCKS, 0), 0x463060: (RIDE, 10), 0x4547f0: (STRIPS, 1), 0x472d10: (HAS, 3), 0x472cb0: (SET_BLOCK, 2), 0x464580: (AFTER, 2), 0x473550: (NONE, 1),
         0x472f40: (SHOW, 1), 0x472ee0: (HIDE, 1), 0x45ba00: (FREE, 1)}
THISCALL = {0x412e90: (FIND, 1)}
HOOKS = {addr: _recorder(k, n, cdecl=True) for addr, (k, n) in CDECL.items()}
HOOKS.update({addr: _recorder(k, n) for addr, (k, n) in THISCALL.items()})
B = Bench([(0x4647a0, 0x464d25), (0x5f5e20, 0x5f5e90), (0x5da3c0, 0x5da3c1)], hooks=HOOKS)
STREAM, VT, DATA, BLOCK, TABLES, RIDES = RAM, RAM + 0x40, RAM + 0x1000, RAM + 0x4000, RAM + 0x7000, RAM + 0x8000
DEFS = RAM + 0x7f00
IDS = 3   # rides, 0x800 bytes of fixture each


def dwords(*values):
    return struct.pack('<%dI' % len(values), *[v & 0xffffffff for v in values])


def ride_fixture(r, regions, i):
    base = RIDES + 0x1000 * i
    held, model, motions = base + 0xc0, base + 0x100, base + 0x180
    verts, first, second, layers, tracks = base + 0x1c0, base + 0x200, base + 0x400, base + 0x600, base + 0x900
    vertex_count, part_first, part_second, layer_count = r.choice([0, 1, 2]), r.choice([0, 1, 2]), r.choice([0, 1]), r.choice([0, 1, 2])
    ride = bytearray(r.bytes(0x100))
    struct.pack_into('<I', ride, 8, held)
    struct.pack_into('<H', ride, 0xe, layer_count)
    struct.pack_into('<I', ride, 0x10, layers)
    for k in range(0xd):
        struct.pack_into('<I', ride, 0x20 + 0xc * k, tracks)
    regions[base] = ride
    h = bytearray(r.bytes(0x30))
    struct.pack_into('<I', h, 4, model)
    struct.pack_into('<I', h, 0x28, r.choice([motions, motions, 0]))
    regions[held] = h
    m = bytearray(r.bytes(0x80))
    struct.pack_into('<HH', m, 0x42, part_first + part_second, part_first)
    struct.pack_into('<H', m, 0x48, vertex_count)
    struct.pack_into('<II', m, 0x70, first, second)
    regions[model] = m
    regions[motions] = dwords(0, verts, 0, 0) + r.bytes(0x10)
    regions[verts] = r.bytes(0x14 * 2)
    regions[first] = r.bytes(0xa0 * 2)
    regions[second] = r.bytes(0x58 * 1)
    regions[layers] = r.bytes(0x38 * 2)
    regions[tracks] = b''.join(dwords(0, r.getrandbits(32) if r.random() < 0.8 else 0) for _ in range(4))
    return vertex_count, part_first + part_second, layer_count


def record(r, shapes, id):
    if r.random() < 0.2:
        return bytes([r.choice([0, 2, 0xff])])
    vertex_count, parts, layers = shapes[id]
    same = r.random() < 0.75
    vs, ps = (vertex_count, parts) if same else r.choice([(vertex_count + 1, parts), (vertex_count, parts + 1), (0, 0), (2, 3)])
    head = bytearray(r.bytes(0x26))
    struct.pack_into('<I', head, 0, r.choice([0x10, 0x20, 0x30, 0x40]))
    struct.pack_into('<II', head, 0xc, r.choice([0, 3, 8]), r.choice([0, 3, 8]))
    counts = bytearray(r.bytes(0x10))
    struct.pack_into('<HH', counts, 4, ps, vs)
    body = r.bytes(8 * vs) + b''.join(dwords(r.getrandbits(12) * r.choice([0, 1])) for _ in range(ps))
    for _ in range(layers):
        layer = bytearray(r.bytes(0x2c))
        struct.pack_into('<III', layer, 0, r.getrandbits(8), r.choice([0, 0, 1, 3, 0xb, 0xc, 0xd]) % 4 if r.random() < 0.6 else 0xc, r.randrange(4))
        body += bytes(layer)
    return b'\x01' + bytes(head) + bytes(counts) + body


def case(r):
    log = log_region()
    log_fold(log)
    regions = {LOG: log, STREAM: dwords(VT, 0, 0, 0), VT: vtable({2: 2})}
    for address, size in ((0x7a4084, 4), (0x7a3ebc, 4), (0x7aedf4, 4), (0x7a2ed8, 8)):
        regions[address] = r.bytes(size)
    shapes = [ride_fixture(r, regions, i) for i in range(IDS)]
    regions[0x7a4610] = dwords(*[RIDES + 0x1000 * i for i in range(IDS)])
    types = r.choice([0, 0, 1, 2, 3])
    regions[0x7a439c] = dwords(types)
    regions[0x7ae6a8] = dwords(*[TABLES + 0x100 * i for i in range(4)])
    for i in range(4):
        regions[TABLES + 0x100 * i] = bytes(0xf8) + dwords(r.choice([0x10, 0x20, 0x30, 0x40, 0x50]))
    id = r.randrange(IDS)   # one ride a case: the answers go by the log's count, not by the record
    records = [record(r, shapes, id) for _ in range(r.choice([0, 1, 1, 2, 3]))]
    first = r.randrange(len(records) + 1)
    data = dwords(first, len(records) - first, r.getrandbits(32)) + b''.join(records)
    whole = dwords(len(data)) + data
    if r.random() < 0.03:
        whole = whole[:r.randrange(len(whole))]
    regions[DATA] = whole + bytes(4)
    regions[BLOCK] = r.bytes(0x2000)
    seq_read(log, READ, DATA, len(whole), buffer=0, count=1)
    strings(log, READ, 0, out=1)
    set_return(log, ALLOC, BLOCK)
    regions[DEFS] = r.bytes(0x20)
    regions[TABLES + 0xf00 - 0x100] = dwords(*[r.choice([0, DEFS, DEFS]) for _ in range(40)])
    answers(log, FIND, TABLES + 0xf00 - 0x100)
    regions[TABLES + 0xc00] = dwords(*([id] * 40))
    answers(log, RIDE, TABLES + 0xc00)
    strings(log, RIDE, 0, out=0)
    blob(log, RIDE, 2, 0xf)
    regions[TABLES + 0xb00] = dwords(*[r.choice([0, 1, 1, 1]) for _ in range(40)])
    answers(log, HAS, TABLES + 0xb00)
    strings(log, HAS, 0, out=1)
    return Call(0x4647a0, [STREAM], regions, ret='a')


if __name__ == '__main__':
    if '--table' in sys.argv:
        for addr, (k, n) in sorted(CDECL.items()):
            print('    {0x%06x, jpeg_recorder(%d, %d)},' % (addr, k, n))
        for addr, (k, n) in sorted(THISCALL.items()):
            print('    {0x%06x, recorder(%d, %d)},' % (addr, k, n))
        sys.exit(0)
    B.main(case)
