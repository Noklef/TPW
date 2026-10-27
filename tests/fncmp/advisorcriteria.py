# The advisor scoring's park criteria (src/game/core/advisorscoring: 0x59c8d0-0x59fb10) against the originals, each on parkenv.py's
# joint park (things, chains, definitions and control records, the analyser, the research lab), the scoring's bounds small (about the
# counts the park can give: -1 to 5, the percentages and half counts about 30 and 100) or anything, its weights anything.
# The criteria of 0x5a02f0-0x5a08b0 (Idle0 ... Share): see late_case(); 0x59f440 / 0x59f590 / 0x59f5e0 / 0x59fb10 (Bus, Stf1f8, Ppl19c,
# StfE): late_case() and late2(). The objects' wear criteria 0x59e450-0x59e680 (Worn ... VWorn5s) and the ring criteria 0x59e730 / 0x59e790
# (RingLo, RingHi): see wear_case(). 0x59ca30 (Obj0w): the things' float +0x44 as for the wear criteria; 0x59cb40 / 0x59cbb0 (MapLo,
# MapHi): the whole map (128 x 128 cells from G's +0x2d8, over the booths' first row) of types 1, 3, 9 and others, +0x2c 0 mostly.
# usage: advisorcriteria.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Booth,No02,...)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import Bench, Call, RAM, put  # noqa: E402
from thingenv import POOL, THING  # noqa: E402
import parkenv  # noqa: E402

# name: (address, the scoring's bounds: offset -> the values about them)
PCT = [0, 25, 33, 50, 50, 66, 100, 100, 150, -1]
WEAR = [0, 0, 1, 10, 30, 50, 70, 90, 99, 100, 101, -1, -50]   # (the objects' average wear, 100 less a byte: -155 to 100)
RING = [0, 1, 2, 3, 5, 50, 100, 200, 255, 256, -1]
CALLS = {'Booth': (0x59c8d0, {}), 'No02': (0x59c960, {}), 'Obj0': (0x59ca80, {}), 'Tal7': (0x59cd30, {}),
         'Low6': (0x59ce30, {0x10c: [0, 1, 2, 30, 100, -1]}), 'Share8': (0x59cea0, {0x114: PCT}), 'All8': (0x59cf50, {}),
         'Tal7b': (0x59d3b0, {}), 'Res0': (0x59da30, {}), 'C14': (0x59dbf0, {0x240: [0, 1, 2, 3, 4, 5, -1]}), 'Res1': (0x59dd40, {}), 'Res2': (0x59dfd0, {}),
         'Res3': (0x59e260, {}), 'TalAb': (0x59e840, {}), 'P210': (0x59f940, {}), 'Mode2': (0x59f970, {}), 'Total': (0x59fa60, {}),
         'Patrol': (0x59fac0, {}),
         'Idle0': (0x5a02f0, {0x57c: [0, 1, 2, 3, -1], 0x580: PCT}), 'Idle1': (0x5a0370, {0x588: PCT}), 'Idle2': (0x5a03d0, {0x590: PCT}),
         'K3F1': (0x5a0430, {0x598: PCT}), 'K3F2': (0x5a0490, {0x5a0: PCT}), 'Tal0': (0x5a04f0, {}), 'TotTh': (0x5a05a0, {}),
         'Mood': (0x5a05f0, {}), 'Money': (0x5a0670, {}), 'One': (0x5a0780, {}), 'CtlLo': (0x5a07d0, {0x5fc: PCT, 0x604: PCT}),
         'CtlMid': (0x5a0820, {0x604: PCT, 0x60c: PCT}), 'CtlHi': (0x5a0870, {0x60c: PCT}), 'Share': (0x5a08b0, {0x614: PCT + [99]}),
         'Bus': (0x59f440, {0x410: [0, 1, 2, 5, -1]}), 'Stf1f8': (0x59f590, {0x42c: [0, 1, 2, 3, -1], 0x430: [0, 10, 50, 100, -1]}),
         'Ppl19c': (0x59f5e0, {0x438: [0, 1, 2, 3, -1], 0x43c: [0, 10, 50, 100, -1]}), 'StfE': (0x59fb10, {0x4d0: [0, 1, 2, 3, -1]}),
         'Worn': (0x59e450, {0x2d0: WEAR, 0x2e0: WEAR}), 'Worn5s': (0x59e510, {0x2d8: WEAR, 0x2e8: WEAR}), 'VWorn': (0x59e5d0, {0x2e0: WEAR}),
         'VWorn5s': (0x59e680, {0x2e8: WEAR}), 'RingLo': (0x59e730, {0x2f0: RING}), 'RingHi': (0x59e790, {0x2f8: RING}),
         'Obj0w': (0x59ca30, {0xc0: [0, 1, 30, 50, 100, 255, 256, -1]}), 'MapLo': (0x59cb40, {0xd8: PCT}), 'MapHi': (0x59cbb0, {0xe0: PCT})}
WORN = {'Worn', 'Worn5s', 'VWorn', 'VWorn5s'}
IDLE = {'Idle0': 0, 'Idle1': 1, 'Idle2': 2, 'K3F1': None, 'K3F2': None}
LATE = {'Idle0', 'Idle1', 'Idle2', 'K3F1', 'K3F2', 'Tal0', 'TotTh', 'Mood', 'Money', 'One', 'CtlLo', 'CtlMid', 'CtlHi', 'Share',
        'Bus', 'Stf1f8', 'Ppl19c', 'StfE'}
B = Bench([(0x412e60, 0x412f00), (0x41a960, 0x41a980), (0x41a990, 0x41a9c0), (0x4c7fa0, 0x4c8080), (0x4c81e0, 0x4c8250), (0x4c9050, 0x4c9080),
           (0x4c91c0, 0x4c92a0), (0x4c94a0, 0x4c94e0), (0x4c9530, 0x4c9680), (0x4c9750, 0x4c9840), (0x4c9960, 0x4c9ab0), (0x4c9b40, 0x4c9c10),
           (0x4c9e60, 0x4c9ef0), (0x4ca310, 0x4ca3b0), (0x4ca450, 0x4ca6c0), (0x4ca7b0, 0x4ca830), (0x4ca8e0, 0x4ca9c0), (0x4d3d80, 0x4d3df0),
           (0x4d8610, 0x4d8690), (0x4e0370, 0x4e0390), (0x4fa990, 0x4faa10), (0x503980, 0x503a00), (0x503b30, 0x503b90), (0x506eb0, 0x506ee0),
           (0x50b350, 0x50b370), (0x516130, 0x5161c0), (0x519510, 0x5195a0), (0x51a280, 0x51a2a0), (0x536300, 0x53644c),
           (0x59c8d0, 0x59c9e0), (0x59ca80, 0x59cad0), (0x59cd30, 0x59cda0), (0x59ce30, 0x59cff0), (0x59d3b0, 0x59d410), (0x59da30, 0x59dae0),
           (0x59dbf0, 0x59dc70), (0x59dd40, 0x59ddf0), (0x59dfd0, 0x59e080), (0x59e260, 0x59e2f0), (0x59e840, 0x59e8a0), (0x59f940, 0x59f9b0),
           (0x59fa60, 0x59fb51), (0x5da3c0, 0x5da3c1),
           (0x409410, 0x40947a), (0x4c7bb0, 0x4c7cb6), (0x4c9130, 0x4c91be), (0x4c9ef0, 0x4ca154), (0x4ca740, 0x4ca7a4),
           (0x4ca9c0, 0x4caa6e), (0x4d09f0, 0x4d09f7), (0x4e0440, 0x4e044b), (0x5195d0, 0x51960d), (0x5aef50, 0x5af0dc),
           (0x5af7f0, 0x5af7fd), (0x5b1680, 0x5b16a6), (0x5b2fd0, 0x5b2feb), (0x5b3110, 0x5b312b), (0x67a830, 0x67a860),
           (0x5a02f0, 0x5a0520), (0x5a05a0, 0x5a06c0), (0x5a0780, 0x5a07b7), (0x5a07d0, 0x5a0930),
           (0x4c7cc0, 0x4c7da9), (0x519450, 0x51948d), (0x59f440, 0x59f4ea), (0x59f590, 0x59f62c),
           (0x59e450, 0x59e7e5), (0x4ca160, 0x4ca214), (0x4ca220, 0x4ca30c), (0x4c9830, 0x4c9912), (0x4c9920, 0x4c9956),
           (0x59ca30, 0x59ca74), (0x59cb40, 0x59cc16), (0x4ca3a0, 0x4ca448), (0x4ccdf0, 0x4ccea6)])
NONE = {'Obj0': (4,), 'Tal7': (7,), 'Low6': (6,), 'Share8': (8,), 'All8': (8,), 'Total': (4, 5, 6, 7, 8), 'Worn': (5,), 'VWorn': (5,),
        'Obj0w': (4,), 'MapLo': (5,), 'MapHi': (5,)}   # (the staff a criterion asks to be absent)
NEED = {'Tal7b': 7, 'Res0': 8, 'C14': 8, 'Res1': 8, 'Res2': 8, 'Res3': 8, 'Worn5s': 5, 'VWorn5s': 5}   # (the kind a criterion asks to be present)
UNFLAGGED = {'Tal7', 'Low6', 'Share8', 'All8', 'Total', 'Idle0', 'Mood', 'CtlLo', 'CtlMid', 'CtlHi', 'Share', 'Worn', 'Worn5s', 'VWorn', 'VWorn5s', 'MapLo', 'MapHi'}
SCORING = RAM + 0x50000
MONEY, SAVE, THEME, MSLOT = RAM + 0x58000, RAM + 0x60000, RAM + 0x60100, 14
ENTRY, ESLOT = RAM + 0x58400, 15   # (StfE's game table entry for G's +0x1da718)
OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


def case(r):
    name = r.deal(OPS)
    address, bounds = CALLS[name]
    deep = r.random() < 0.6   # (mostly the staff and the flag as a criterion asks, and the park rich: to reach the last tests)
    kinds = parkenv.KINDS
    if deep:
        kinds = tuple(k for k in kinds if k not in NONE.get(name, ())) + (NEED[name],) * 6 if name in NEED else \
            tuple(k for k in kinds if k not in NONE.get(name, ()))
    if deep and name in LATE:
        kinds = kinds + (3, 3, 3, 1, 1, 1, 1, 1)   # (more objects and people to share out)
    if deep and name in WORN:
        kinds = kinds + (3, 3, 3)
    regions, _info = parkenv.park(r, kinds=kinds, flag=0 if deep and name in UNFLAGGED else None, rich=deep)
    s = bytearray(r.bytes(0x620 if name in LATE else 0x4d0))
    for off in range(0xa0, 0x4d0, 4):   # (most dwords small, so a bound lands about the counts; the weights small or anything)
        if r.random() < 0.7:
            put(s, off, 'I', parkenv.small(r))
    for off, values in bounds.items():
        if r.random() < 0.8:
            put(s, off, 'I', r.choice(values) & 0xffffffff)
    regions[SCORING] = s
    if name in LATE:
        late_case(r, regions, _info, name)
    if name in WORN or name in ('RingLo', 'RingHi', 'Obj0w', 'MapLo', 'MapHi'):
        wear_case(r, regions, _info, name)
    return Call(address, [SCORING], regions, conv='t', ret='a')


def wear_case(r, regions, info, name):
    """The wear criteria: the objects' (kind 3) byte +0x32 bit 0 set mostly and their float +0x44 0-100 mostly (else past 255, negative, NaN);
    the ring criteria: the analyser's ring 0 (+0x216fc) full mostly (its size +0x38 or cursor +0x34 about 50, wrapped +0x3c or not), its
    bytes any, small or all one value, the bound about them."""
    if name in ('MapLo', 'MapHi'):
        empty = r.random() < 0.1
        types = [2, 7, 0x1e, 0, 0xa] if empty else [2, 7, 0x1e, 0, 1, 3, 9, 0xa]
        cells = bytearray(regions[parkenv.BOOTHS]) + bytearray(r.bytes(0x44 + 0x44 * 0x4000 - len(regions[parkenv.BOOTHS])))
        for c in range(0x4000):
            put(cells, 0x44 * (c + 1) + 8, 'I', r.choice(types) if r.random() < 0.9 else r.getrandbits(32))
            if empty or r.random() < 0.7:
                put(cells, 0x44 * (c + 1) + 0x2c, 'I', 0)
        regions[parkenv.BOOTHS] = cells
        return
    if name in WORN or name == 'Obj0w':
        pool = bytearray(regions[POOL])
        for i in range(len(info['things'])):
            if pool[THING * i + 2] == 3 or name == 'Obj0w':
                pool[THING * i + 0x32] = r.choice([1, 1, 1, 3, 0, 2, r.randrange(256)])
                put(pool, THING * i + 0x44, 'f', r.choice([0.0, 10.0, 50.5, 99.9, 100.0, r.uniform(0, 100), r.uniform(0, 100), 300.7, -5.5,
                                                           float('nan')]))
        regions[POOL] = pool
        return
    ring = bytearray(r.bytes(0x40))
    put(ring, 0x38, 'i', r.choice([50, 50, 51, 60, 49, 0, -1, r.randrange(-3, 60)]))
    put(ring, 0x34, 'i', r.choice([49, 49, 48, 50, 10, -1, r.randrange(-3, 60)]))
    ring[0x3c] = r.choice([0, 1, 1, 0x80])
    kind = r.choice(['any', 'small', 'one'])
    v = r.randrange(256)
    ring[1:0x34] = r.bytes(0x33) if kind == 'any' else bytes(r.randrange(8) for _ in range(0x33)) if kind == 'small' else bytes([v]) * 0x33
    regions[parkenv.ANALYSER + 0x216fc] = ring
    if kind == 'one' and r.random() < 0.5:
        bound = 0x2f0 if name == 'RingLo' else 0x2f8
        regions[SCORING] = put(bytearray(regions[SCORING]), bound, 'i', v + r.choice([-1, 0, 1]))


def late_case(r, regions, info, name):
    """The criteria of 0x5a02f0-0x5a08b0 on case()'s park and scoring (0x620 bytes for them), with: the things' byte +0x32 any of bits 0-1 and
    their +0x68 0 or not (the kind 3 counts and the idle ones), for Mood the people's +0x19c's low byte 0-100 mostly (the average
    Things_AveragePersonStat19c takes), for Idle0-2 and K3F1-2 half the things of other kinds made
    objects (kind 3) and for Idle0-2 the definitions' +0x4ac the criterion's value mostly; the criteria's weights nonzero mostly; the analyser's recount fields (+0x1f0dc-0x1f0e5) and its count at
    +0x216d8 (+0x18 0 or -1 mostly, +0x1c 1 or not, the byte +0x20 set or not); a money record in the game table's slot 14 (G's word
    +0x1da726) with its total +0x124 small or anything; the global save info [0xf7c58c] (never 0: GlobalSave_Instance would build
    one) with a theme record (+0x48) whose bytes 0-4 are 0 mostly; the globals 0x785790-0x7857a0 about the counts (0x785794 0 or
    negative now and then) and the scoring's floats +0x5c4 / +0x5cc / +0x5d4 small multipliers (now and then NaN or negative). For Mood
    now and then a tie (every person's byte v, the bound v * 1.0); for Money half the time a large product (2^24-2^30 times 0.1-1.9) with
    the total within 40 of it (where the 24-bit fmul and a double one differ); Share's bound 99 now and then (the share's 100 cap)."""
    pool = bytearray(regions[POOL])
    for i in range(len(info['things'])):
        pool[THING * i + 0x32] = r.choice([0, 1, 2, 3, r.randrange(256)])
        put(pool, THING * i + 0x68, 'I', r.choice([0, 0, 0, 1, r.getrandbits(32)]))
        if name == 'Mood' and pool[THING * i + 2] == 1 and r.random() < 0.8:
            pool[THING * i + 0x19c] = r.randrange(101)
        if name in IDLE and pool[THING * i + 2] not in (1, 3) and r.random() < 0.5:
            pool[THING * i + 2] = 3
    if IDLE.get(name) is not None:   # (the definitions' +0x4ac the criterion's value mostly)
        for i in range(8):
            if parkenv.ITEMS + parkenv.ITEM * i + 0x4ac in regions and r.random() < 0.6:
                regions[parkenv.ITEMS + parkenv.ITEM * i + 0x4ac] = struct.pack('<I', IDLE[name])
    regions[POOL] = pool
    s = bytearray(regions[SCORING])
    for off in range(0x57c, 0x620, 4):
        if r.random() < 0.7:
            put(s, off, 'I', parkenv.small(r, 0x32, 0x64))
    for off, values in CALLS_LATE_BOUNDS.items():
        if r.random() < 0.6:
            put(s, off, 'I', r.choice(values) & 0xffffffff)
    for off in (0x584, 0x58c, 0x594, 0x59c, 0x5a4, 0x5ac, 0x5c8, 0x5d0, 0x5d8, 0x5f4, 0x600, 0x608, 0x610, 0x618):
        if r.random() < 0.9:
            put(s, off, 'I', r.choice([1, 5, 100, r.getrandbits(32) | 1]))
    for off in (0x5c4, 0x5cc, 0x5d4):
        put(s, off, 'f', r.choice([0.0, 0.5, 1.0, 1.0, 2.0, 0.25, 30.0, -1.0, float('nan'), r.uniform(0, 3)]))
    regions[SCORING] = s
    kept = bytearray(regions[parkenv.ANALYSER + 0x1f0b0]) + bytearray(r.bytes(0x10))
    regions[parkenv.ANALYSER + 0x1f0b0] = kept
    count = bytearray(r.bytes(0x24))
    put(count, 0x18, 'I', r.choice([0, 0xffffffff, 1, 2, r.getrandbits(32)]))
    put(count, 0x1c, 'I', r.choice([1, 1, 0, 2, r.getrandbits(32)]))
    count[0x20] = r.choice([0, 0, 1, 0x80])
    regions[parkenv.ANALYSER + 0x216d8] = count
    slots = bytearray(regions[parkenv.SLOTS])
    put(slots, 20 * MSLOT, 'I', MONEY)
    regions[parkenv.SLOTS] = slots
    tail = bytearray(regions[parkenv.TAIL])
    put(tail, 0x2a, 'H', MSLOT)
    regions[parkenv.TAIL] = tail
    regions[MONEY + 0x124] = struct.pack('<I', parkenv.small(r, 100, 1000, 5000, 0x7fffffff))
    regions[SAVE] = put(bytearray(r.bytes(0x50)), 0x48, 'I', THEME)
    regions[THEME] = bytes(r.choice([0, 0, 0, 1, 0xff]) for _ in range(8))
    regions[0xf7c58c] = struct.pack('<I', SAVE)
    regions[0x785790] = struct.pack('<5I', parkenv.small(r, 10, 50, 100), r.choice([1, 2, 5, 10, 0x7fffffff, 0xffffffff, 0xfffffffb]),
                                    r.choice([0, 1, 10, 30, 0xffffffff]), r.choice([0, 0, 1, 2, 0xffffffff]), parkenv.small(r, 10, 50, 100))
    if r.random() < 0.2:
        regions[0x785790] = put(bytearray(regions[0x785790]), 4, 'I', 0)
    if name in LATE2:
        late2(r, regions, info, name, pool)
    if name == 'Mood' and r.random() < 0.35:   # (a tie: every person's byte v, the bound v * 1.0)
        v = r.randrange(101)
        for i in range(len(info['things'])):
            if pool[THING * i + 2] == 1:
                pool[THING * i + 0x19c] = v
        regions[POOL] = pool
        regions[0x785790] = put(bytearray(regions[0x785790]), 8, 'I', v)
        regions[SCORING] = put(bytearray(regions[SCORING]), 0x5cc, 'f', 1.0)
    if name == 'Money' and r.random() < 0.5:   # (the total about a large product, where the 24-bit fmul and a double one part)
        n, f = r.randrange(1 << 24, 1 << 30), struct.unpack('<f', struct.pack('<f', r.uniform(0.1, 1.9)))[0]
        regions[0x785790] = put(bytearray(regions[0x785790]), 0x10, 'I', n)
        regions[SCORING] = put(bytearray(regions[SCORING]), 0x5d4, 'f', f)
        regions[MONEY + 0x124] = struct.pack('<I', int(n * f) + r.randrange(-40, 41))



def late2(r, regions, info, name, pool):
    """Bus: the analyser's count buffer +0x216d8 (5 samples from +4) with its cursor -2..4 and size -1..5 mostly, the samples about the
    bound +0x410, and the bus stops 0x7855ac (A x, y, B x, y). Stf1f8 / Ppl19c: the staff's +0x1f8 / the people's +0x19c low bytes 0-100,
    now and then all one value v with the bound +0x430 / +0x43c v (a tie). StfE: G's +0x1da718 the slot 15 mostly, an entry there
    whose byte +0x20 is 0 or not."""
    s = bytearray(regions[SCORING])
    if name == 'Bus':
        count = bytearray(r.bytes(0x24))
        put(count, 4, '5i', *(r.choice([0, 1, 2, 3, 5, -1, r.getrandbits(31)]) for _ in range(5)))
        put(count, 0x18, 'i', r.choice([-1, -1, 0, 1, 2, 3, 4, -2, -5]))
        put(count, 0x1c, 'i', r.choice([1, 2, 3, 4, 5, 5, 0, -1]))
        count[0x20] = r.choice([0, 1, 1, 0x80])
        regions[parkenv.ANALYSER + 0x216d8] = count
        regions[0x7855ac] = struct.pack('<4I', *(r.choice([0, 1, 5, 0x7f, 0xffff, 0x10000 + r.randrange(0x100), r.getrandbits(32)]) for _ in range(4)))
    elif name in ('Stf1f8', 'Ppl19c'):
        staff, off, bound = (lambda k: 4 <= k <= 8, 0x1f8, 0x430) if name == 'Stf1f8' else (lambda k: k == 1, 0x19c, 0x43c)
        tie = r.randrange(101) if r.random() < 0.35 else None
        for i in range(len(info['things'])):
            if staff(pool[THING * i + 2]):
                pool[THING * i + off] = tie if tie is not None else r.choice([r.randrange(101), r.randrange(256)])
        regions[POOL] = pool
        if tie is not None:
            put(s, bound, 'i', tie)
    elif name == 'StfE':
        slots = bytearray(regions[parkenv.SLOTS])
        slots += bytearray(max(0, 20 * (ESLOT + 1) - len(slots)))
        put(slots, 20 * ESLOT, 'I', ENTRY)
        regions[parkenv.SLOTS] = slots
        tail = bytearray(regions[parkenv.TAIL])
        put(tail, 0x1c, 'H', ESLOT if r.random() < 0.95 else MSLOT)
        regions[parkenv.TAIL] = tail
        regions[ENTRY] = bytes(0x20) + bytes([r.choice([0, 0, 1, 0x80])]) + r.bytes(3)
    put(s, {'Bus': 0x414, 'Stf1f8': 0x434, 'Ppl19c': 0x440, 'StfE': 0x4d4}[name], 'I', r.choice([1, 5, 100, r.getrandbits(32) | 1]))
    regions[SCORING] = s


LATE2 = {'Bus', 'Stf1f8', 'Ppl19c', 'StfE'}
CALLS_LATE_BOUNDS = {0x598: PCT, 0x5a0: PCT, 0x5a8: [0, 1, 2, -1], 0x5fc: PCT, 0x604: PCT, 0x60c: PCT, 0x614: PCT + [99, 99]}


if __name__ == '__main__':
    B.main(case)
