# The counts over the object definitions and the world's objects (src/game/world/objectdefs: 0x4ca4c0, 0x4ca510, 0x4ca570, 0x4ca5d0,
# 0x4ca450, 0x4ca7b0; src/game/world/object: 0x4ca310) against the originals: up to 12 definitions (0x7890a0: ids among a few, repeated;
# map kinds +0x4ac 0-3 mostly; +0xc4 0 or not; the walk cursor anything), the game's control records (at +0x1d943c of [0x80239c]: one for
# each id mostly, their byte +0x10 and dwords +0x14 / +0x18 about 0, signed; the cache hit or not; a missing record a fault in both), the
# flag's low byte set or not (its high bytes noise) and the kind 0-4. Objects: the list from the world's +0x1da746 (up to 8, the slots at
# 0x7cfb90, linked by +0xc) of ids among the definitions' (one missing now and then: a fault in both), in state (+0x19c) 1 or not, the
# definitions' find cache (+0x730 / +0x734) hit or not. Below44 (0x4ca3a0): Objects' list, the objects' floats +0x44 0..100 mostly (else
# negative, past 255 or NaN), the bound about them (or negative or past 255).
# usage: objectcounts.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Kind0At14,Kind1,Kind2,Kind3,Kind0At10,KindAt18,Objects,Below44)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import RAM, Bench, Call, put  # noqa: E402

CALLS = {'Kind0At14': 0x4ca4c0, 'Kind1': 0x4ca510, 'Kind2': 0x4ca570, 'Kind3': 0x4ca5d0, 'Kind0At10': 0x4ca450, 'KindAt18': 0x4ca7b0,
         'Objects': 0x4ca310, 'Below44': 0x4ca3a0}
OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]
B = Bench([(0x4ca4c0, 0x4ca50e), (0x4ca510, 0x4ca561), (0x4ca570, 0x4ca5c1), (0x4ca5d0, 0x4ca621), (0x4ca450, 0x4ca4b6), (0x4ca7b0, 0x4ca81a),
           (0x4ca310, 0x4ca39f), (0x412e60, 0x412e6d), (0x412e70, 0x412e8f), (0x412e90, 0x412eea), (0x4d3d80, 0x4d3ddd), (0x4e0370, 0x4e037f),
           (0x5da3c0, 0x5da3c1), (0x4ca3a0, 0x4ca448)])
GAME = 0x1d943c   # (the control table's offset in the game object)
TABLE, ITEMS, OBJS = RAM, RAM + 0x4000, RAM + 0x8000
DEFS = 0x7890a0
IDS = [1, 2, 3, 4, 5, 9]
SIZE = 0x500   # (a definition's room)


def s32(r, rare=0x80000000):
    return r.choice([0, 1, 7, 7, -1, rare]) & 0xffffffff


def case(r):
    op = r.deal(OPS)
    n = r.choice([1, 2, 5, 12, 12] if op in ('Objects', 'Below44') else [0, 2, 5, 12, 12, 12])
    defs,items = bytearray(r.bytes(0x740)), bytearray(r.bytes(12 * SIZE))
    put(defs, 0x528, 'i', n)
    put(defs, 0x72c, 'i', r.choice([-1, 0, 3, 11]))
    put(defs, 0x730, 'H', r.choice(IDS + [0, 0x77]))
    put(defs, 0x734, 'I', ITEMS + SIZE * r.randrange(12))
    for i in range(12):
        put(defs, i * 4, 'I', ITEMS + SIZE * i)
        put(items, SIZE * i, 'H', r.choice(IDS))
        put(items, SIZE * i + 0x4ac, 'I', r.choice([0, 0, 1, 2, 3, 4, 0x100]))
        put(items, SIZE * i + 0xc4, 'I', s32(r))
    table = bytearray(r.bytes(0x1400))
    ids = r.sample(IDS, len(IDS))
    if r.random() < 0.05:
        ids.pop()
    put(table, 0x12c0, 'i', len(ids))
    for k, i in enumerate(ids):
        put(table, 0x20 * k, 'H', i)
        table[0x20 * k + 0x10] = r.choice([0, 1, 1, 0x80])
        put(table, 0x20 * k + 0x14, 'I', s32(r))
        put(table, 0x20 * k + 0x18, 'I', s32(r))
    put(table, 0x12c4, 'H', r.choice(IDS + [0, 0x77]))
    put(table, 0x12c8, 'I', TABLE + 0x20 * r.randrange(len(ids)))
    regions = {TABLE: table, DEFS: defs, ITEMS: items, 0x80239c: (TABLE - GAME).to_bytes(4, 'little')}
    if op in ('Objects', 'Below44'):
        known = [i for i in IDS if any(int.from_bytes(items[SIZE * j:SIZE * j + 2], 'little') == i for j in range(n))]
        handles = r.sample(range(1, 20), r.choice([0, 1, 3, 5, 8, 8]))
        slots = bytearray(r.bytes(20 * 20))
        for k, h in enumerate(handles):
            obj = bytearray(r.bytes(0x1a0))
            put(obj, 0xc, 'H', handles[k + 1] if k + 1 < len(handles) else 0)
            put(obj, 0xe, 'H', r.choice(known) if known and r.random() < 0.97 else r.choice(IDS))
            put(obj, 0x19c, 'I', r.choice([1, 1, 1, 0, 2, 0x101]))
            if op == 'Below44':
                put(obj, 0x44, 'f', r.choice([0.0, 1.0, 20.5, 50.0, 99.9, 100.0, r.uniform(0, 100)]) if r.random() < 0.85 else
                    r.choice([-1.0, -0.5, 255.9, 256.0, 300.0, float('nan'), 1e10]))
            regions[OBJS + 0x200 * k] = obj
            put(slots, 20 * h, 'I', OBJS + 0x200 * k)
        put(slots, 0, 'I', r.choice([0, OBJS]))   # (only asserted on)
        regions[0x7cfb90] = slots
        put(table, 0x1da746 - GAME, 'H', handles[0] if handles else 0)
        if not known:   # (the find cache then the only way to a definition)
            put(defs, 0x734, 'I', ITEMS)
        if op == 'Below44':
            bound = r.choice([0, 1, 20, 50, 50, 100, 101, 255, 256, r.randrange(101), -1, r.getrandbits(32)])
            return Call(CALLS[op], [bound & 0xffffffff], regions, conv='s', ret='a')
        return Call(CALLS[op], [], regions, conv='c', ret='a')
    flag = r.choice([0, 1, 0x100, 0xff00, 0x80000001])
    if op == 'KindAt18':
        return Call(CALLS[op], [r.choice([0, 0, 1, 2, 3, 4]), flag], regions, conv='s', ret='a')
    return Call(CALLS[op], [flag if op == 'Kind0At10' else r.getrandbits(32)], regions, conv='s', ret='a')


if __name__ == '__main__':
    B.main(case)
