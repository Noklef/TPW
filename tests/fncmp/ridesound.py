# A ride's linked sound played (src/game/core/soundplay: 0x51eeb0) against the original: ridelist.py's records (0-4 of the ids 1-8, each
# linking one of them or 99; their arrays' elements the sounds, 0 now and then), the id one of them or not and the index 0-5 (now and
# then past the array); soundplay.py's sound globals and manager (its command logged as a hash of its 7 dwords), the owner, category and
# position any.
# usage: ridesound.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Play)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, blob, log_region, put, set_return, vtable  # noqa: E402
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import particles  # noqa: E402
import ridebounds  # noqa: E402

CALLS = {'Play': 0x51eeb0, 'Effect': 0x55a100}
OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]
B = Bench([(0x51eeb0, 0x51eeec), (0x55a3e0, 0x55a470), (0x55a070, 0x55a0b0), (0x51bfc0, 0x51c048), (0x5da3c0, 0x5da3c1),
           (0x55a100, 0x55a2f5), (0x412e90, 0x412eea), (0x466b70, 0x466c90), (0x520030, 0x5200a4)] + particles.RANGES)
PNODES, PSINES, RIDEWORLD, THING, DEFS = RAM + 0x6000, RAM + 0x7000, RAM + 0x10000, RAM + 0x8000, RAM + 0x9000
NODES, ARRAYS, MANAGER, VT = RAM + 0x1000, RAM + 0x4000, RAM + 0x100, RAM + 0x200


def coord(r):
    return r.choice([0, 1, -1, r.randrange(-2000, 2000), r.getrandbits(32) - (1 << 31)]) & 0xffffffff


def case(r):
    op = r.deal(OPS)
    count = r.choice([0, 1, 2, 3, 4, 4, 4, 4]) if op == 'Play' else (0 if r.random() < 0.02 else r.choice([1, 2, 3, 4, 4]))
    ids = r.sample(range(1, 9), count)
    nodes, arrays = bytearray(r.bytes(0x100 * 4)), bytearray(r.bytes(0x40 * 4))
    for i, rid in enumerate(ids):
        o = 0x100 * i
        put(nodes, o, 'I', NODES + 0x100 * (i + 1) if i + 1 < count else 0)
        put(nodes, o + 8, 'I', rid)
        put(nodes, o + 0x14, 'I', r.choice(ids + ids + [99]))
        put(nodes, o + 0x1c, 'I', ARRAYS + 0x40 * i)
        put(nodes, o + 0x8c, 'i', r.choice([2, 4, 6, 6, 6, 6, 0, -1]))
        for k in range(6):
            put(arrays, 0x40 * i + 4 * k, 'I', r.choice([0, 1, 0x1d, 0x123, r.getrandbits(32)]))
    log = log_region()
    g = bytearray(r.bytes(0x20))   # 0x802bc0..
    put(g, 8, 'I', r.choice([0, 1, 1, 1, 1]))
    put(g, 0xc, 'I', MANAGER)
    put(g, 0x14, 'I', r.choice([0, 1, 1, 1, 1]))
    blob(log, 2, 1, 0x7f)
    if r.random() < 0.85:
        set_return(log, 2, r.getrandbits(32))
    regions = {NODES: nodes, ARRAYS: arrays, 0x8791a0: struct.pack('<I', r.choice([0, 1, 1, 1, 1])),
               0x8791b0: struct.pack('<I', NODES if count else 0), LOG: log, 0x802bc0: g,
               MANAGER: struct.pack('<I', VT) + r.bytes(0xc), VT: vtable({2: 2}, 4)}
    if op == 'Effect':
        return effect_case(r, regions, ids)
    ident = r.choice(ids + ids + [0, 99]) if ids else r.choice([0, 1, 5])
    index = r.choice([0, 1, 2, 3, 4, 5, 5, -1, 0x7fffffff])
    return Call(CALLS['Play'], [ident, r.getrandbits(32), r.getrandbits(32), index & 0xffffffff, coord(r), coord(r), coord(r)], regions, ret='a')


def effect_case(r, regions, ids):
    """0x55a100 (id): case()'s records and sound (the system on mostly, the id one of the records' mostly: none reads through null in
    both), each record's thing (+0xac) 0 now and then, else one whose type (+0xe) has a definition (0x7890a0: up to three, cached or
    not) with an effect (+0x6c) 0 now and then; the ride index +0xc8 ridebounds.box()'s ride (at RIDEWORLD), 0 or another (no
    box); particles.spawn()'s world (its sines and nodes moved clear of the records)."""
    c = ridebounds.box(r, RIDEWORLD)
    index = c.args[0]
    regions.update(c.regions)
    nodes = bytearray(regions[NODES])
    for i in range(len(ids)):
        put(nodes, 0x100 * i + 0xac, 'H', r.choice([0, 1, 1, 1, 2]))
        put(nodes, 0x100 * i + 0xc8, 'I', index if r.random() < 0.85 else r.choice([0, 0, 0, r.randrange(8)]))
    regions[NODES] = nodes
    if r.random() < 0.9:
        regions[0x8791a0] = struct.pack('<I', 1)
    types = [r.getrandbits(16) for _ in range(3)]
    ndefs = 0 if r.random() < 0.02 else r.choice([1, 2, 3, 3])
    defs = bytearray(r.bytes(0x738))
    blocks = bytearray(r.bytes(0x300))
    for k in range(3):
        put(defs, 4 * k, 'I', DEFS + 0x100 * k)
        put(blocks, 0x100 * k, 'H', types[k])
        put(blocks, 0x100 * k + 0x6c, 'I', r.choice([0, 0x13, 0x27, r.randrange(0x69), r.randrange(0x69), r.getrandbits(32)]))
    put(defs, 0x528, 'i', ndefs)
    if r.random() < 0.3:
        k = r.randrange(3)
        put(defs, 0x730, 'HxxI', types[k], DEFS + 0x100 * k)
    things = bytearray(r.bytes(60))
    for h in (1, 2):
        put(things, 20 * h, 'I', THING + 0x20 * h)
    thing = bytearray(r.bytes(0x60))
    for h in (1, 2):
        put(thing, 0x20 * h + 0xe, 'H', r.choice(types[:max(ndefs, 1)] + [types[0]]))
    p = dict(particles.spawn(r).regions)
    p[PSINES], p[PNODES] = p.pop(particles.SINES), p.pop(particles.NODES)
    p[0x80ced0] = put(put(bytearray(p[0x80ced0]), 0x1c, 'I', PSINES), 0x28, 'I', PNODES)
    regions.update(p)
    regions.update({0x7890a0: defs, DEFS: blocks, 0x7cfb90: things, THING: thing, 0x803a28: r.bytes(4)})
    ident = r.choice(ids) if ids and r.random() < 0.98 else 99
    return Call(CALLS['Effect'], [ident], regions, ret='v')


if __name__ == '__main__':
    B.main(case)
