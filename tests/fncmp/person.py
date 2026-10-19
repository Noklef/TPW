# The things' map ids and the people's small methods (src/game/world/person: 0x4e2650, 0x4f9460,
# 0x4fd950, 0x4fdc60, 0x501290, 0x502430, 0x4fec10, 0x4fe9c0, 0x4fea70, 0x4fb4f0) and the thoughts' constructor (0x50ba80) against the
# originals: positions anywhere, outs over the fields, ticks about the wrap; Raise / Lower / Add100: the global bytes and the floats as below.
# usage: person.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=MapId,Record,TimeInPark,PushRide,State11,State11To14,Thoughts,State6To10,Raise,Lower,Add100)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, fb, put  # noqa: E402

B = Bench([(0x4e2650, 0x4e266d), (0x4f9460, 0x4f948b), (0x4fd950, 0x4fd967), (0x4fdc60, 0x4fdc89), (0x501290, 0x5012ac),
           (0x502430, 0x502454), (0x50ba80, 0x50bab0), (0x4fe9c0, 0x4fea62), (0x4fea70, 0x4feb0a), (0x4fec10, 0x4fec48), (0x4fb4f0, 0x4fb52b),
           (0x5da3c0, 0x5da3c1)])
THING, WORLD = RAM, RAM + 0x1000
GWORLD = 0x80239c
CALLS = {'MapId': 0x4e2650, 'Record': 0x4f9460, 'TimeInPark': 0x4fd950, 'PushRide': 0x4fdc60, 'State11': 0x501290, 'State11To14': 0x502430, 'Thoughts': 0x50ba80,
         'State6To10': 0x4fec10, 'Raise': 0x4fe9c0, 'Lower': 0x4fea70, 'Add100': 0x4fb4f0}


def word(r):
    return r.choice([0, 1, 4, 0x7fffffff, 0x80000000, 0xffffffff, r.getrandbits(32)])


def stat(r):
    """A float about 0..100 (bits): zeros of both signs, the ends and past them, denormals, infinities, NaNs (now and then signalling)."""
    v = r.choice([0.0, -0.0, 0.5, 1.0, 50.0, 99.0, 99.75, 100.0, 100.5, -0.5, -1.0, -300.0, 300.0, 1e-40, -1e-40, 3e38, float('nan'),
                  float('inf'), float('-inf'), r.uniform(-10, 110), r.uniform(-10, 110), r.uniform(0, 100), struct.unpack('<f', r.bytes(4))[0]])
    return fb(v) if r.random() < 0.97 else 0x7fa00000 | r.getrandbits(20)


def case(r):
    op = r.choice(OPS)
    thing = bytearray(r.bytes(0x240))
    regions = {THING: thing}
    if op == 'TimeInPark':
        world = bytearray(r.bytes(0x10))
        tick = word(r)
        put(world, 0, 'I', tick)
        put(thing, 0x1d4, 'I', r.choice([tick, (tick - r.randrange(64)) & 0xffffffff, word(r)]))
        regions[WORLD] = world
        regions[GWORLD] = bytearray((WORLD - 0x1da70c & 0xffffffff).to_bytes(4, 'little'))
        return Call(0x4fd950, [THING], regions, conv='f', ret='a')
    if op in ('State11', 'State11To14'):
        states = [7, 8, 10, 11, 12, 14, 15, 0xffffffff, 0x8000000b, r.getrandbits(32)]
        put(thing, 0x220, 'II', r.choice(states), r.choice(states))
        thing[0x1f1] = r.choice([0, 0, 1, 0xff, r.getrandbits(8)])
        return Call(CALLS[op], [THING], regions, conv='f', ret='a')
    if op == 'State6To10':
        put(thing, 0x220, 'I', r.choice([5, 6, 7, 8, 9, 10, 11, 0, 0x80000006, 0x106, r.getrandbits(32)]))
        return Call(CALLS[op], [THING], regions, conv='t', ret='a')
    if op == 'Add100':   # the float and delta as Raise's float, the delta now and then whole and about -100..100
        put(thing, 0x1b8, 'I', stat(r))
        delta = stat(r) if r.random() < 0.6 else fb(float(r.randrange(-120, 121)))
        return Call(CALLS[op], [THING + 0x1b8, delta], regions, conv='t', ret='v')
    if op in ('Raise', 'Lower'):   # the bytes 0x785058 / 0x78505c / 0x785060 any, the index 0-2 or past, the float about 0..100 (NaN and the rest)
        regions[0x785058] = bytes(r.choice([0, 1, 5, 50, 100, 255, r.getrandbits(8)]) if k % 4 == 0 else r.getrandbits(8) for k in range(12))
        put(thing, 0x19c, 'I', stat(r))
        return Call(CALLS[op], [THING, r.choice([0, 1, 2, 0, 1, 2, 3, 0xffffffff, r.getrandbits(32)])], regions, conv='t', ret='v')
    if op == 'Thoughts':
        return Call(0x50ba80, [THING + r.randrange(16)], regions, conv='t', ret='a')
    if op == 'PushRide':
        return Call(0x4fdc60, [THING, r.getrandbits(32)], regions, conv='t', ret='v')
    out = THING + r.choice([0x1f0 + r.randrange(8), 0xcb, 0xcc, 0xcd, 4, 6])  # aliasing the field or the position
    return Call(CALLS[op], [THING, out], regions, conv='t', ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
