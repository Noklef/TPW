# The flying rides module's save game read (src/game/core/savemods: FlyingRides_Load 0x55e020) against the original, with FlyingRide_Free's
# world (flyingclear.py's Free: one record at an index, its slots in use or not, their parts, systems, rides and sounds) and FlyingRides_Init
# run. The module on (mostly) or off before: on, its sines block any and the table empty but for that record and now and then a second one
# with no slot in use. The stream (a record stream) as FlyingRides_Save writes it: 'FLY_' (mostly), the flag (0 or not), the table's size
# (400 mostly; 0 or 8) and bytes (entries 1-99 empty but for 0-3 any), the four dwords 0x7660b8, five pads and the record size (0x122c
# mostly; 0 or 0x10), a record of that size for each entry. The stream whole, or cut short past the magic (before it the original's slot
# would be uninitialised stack). Shutdown (0x55d4d0 FlyingRides_Shutdown): the same world, no stream.
# usage: flyingload.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Load,Shutdown)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import HEAP, RAM, Bench, Call, heap, put, record_stream  # noqa: E402
_ops = os.environ.pop('OPS', None)   # (the imported benches read OPS as their own)
import flyingclear as fc  # noqa: E402
if _ops is not None:
    os.environ['OPS'] = _ops

B = Bench(fc.merged(fc.RANGES + [(0x55e020, 0x55e329), (0x55a5a0, 0x55a61f), (0x5f5660, 0x5f56a0), (0x5f5e20, 0x5f5e50), (0x67a830, 0x67a860),
                                 (0x5da3c0, 0x5da3c1), (0x55d4d0, 0x55d608), (0x55e330, 0x55e3a8)]), hooks=fc.HOOKS)
STREAM, SINK, REC2 = RAM + 0x40000, RAM + 0x41000, RAM + 0x46000
MODULE = 0x8791c8   # the sines block, ..., the flag 0x8791ec, 0x8791f0, the table 0x8791f8 (100 entries)


def image(r):
    d = bytearray(b'FLY_' if r.random() < 0.97 else r.bytes(4))
    safe = len(d)
    flag = r.choice([1, 1, r.getrandbits(32), 0, 0, 0, 0, 0])   # (set: 4096 fsins in FlyingRides_Init, the bench's time)
    d += struct.pack('<I', flag)
    if flag:
        size = r.choice([400] * 8 + [0, 8])
        table = bytearray(400)
        put(table, 0, 'I', r.getrandbits(32))
        entries = r.sample(range(1, 100), r.choice([0, 1, 1, 2, 3]))
        if entries and r.random() < 0.5:   # (the first and last entries now and then: the walk's ends)
            entries[0] = r.choice([1, 99])
            entries = sorted(set(entries))
        for i in entries:
            put(table, 4 * i, 'I', r.getrandbits(32) | 1)
        d += struct.pack('<I', size) + table[:size] + r.bytes(16) + b'PAD_' * 5
        block = r.choice([0x122c] * 6 + [0, 0x10])
        d += struct.pack('<I', block)
        for _ in entries if size == 400 else ():
            d += r.bytes(block)
    return bytes(d), safe


def case(r):
    regions = dict(fc.free(r).regions)
    old = bytearray(regions.pop(0x8791f8))
    module = bytearray(r.bytes(0x30)) + bytearray(0x194)
    put(module, 0x24, 'I', r.choice([1, 1, 1, r.getrandbits(32), 0]))
    for i in range(1, 100):   # (the record free() placed, at its index)
        if struct.unpack_from('<I', old, 4 * i)[0] == fc.REC:
            put(module, 0x30 + 4 * i, 'I', fc.REC)
    if r.random() < 0.3:
        rec2 = bytearray(r.bytes(0x5c + 20 * 0xe4))
        for i in range(20):
            put(rec2, 0x5c + i * 0xe4 + 0x80, 'I', 0)
        free = [i for i in range(1, 100) if not struct.unpack_from('<I', module, 0x30 + 4 * i)[0]]
        put(module, 0x30 + 4 * r.choice(free), 'I', REC2)
        regions[REC2] = rec2
    regions.update({MODULE: module, HEAP: heap(0x8000)})
    if r.choice(OPS) == 'Shutdown':
        return Call(0x55d4d0, [], regions, ret='v')
    data, safe = image(r)
    if r.random() < 0.15:
        data = data[:r.randrange(safe, len(data) + 1)]
    regions.update({0x7660b0: r.bytes(0x20), STREAM: record_stream(STREAM, SINK, len(data)), SINK: data + bytes(0x40)})
    return Call(0x55e020, [STREAM], regions, ret='a')


OPS = [k for k in os.environ.get('OPS', 'Load,Shutdown').split(',') if k]


if __name__ == '__main__':
    B.main(case)
