# The things' map ids and the people's small methods (src/game/world/person: 0x4e2650, 0x4f9460,
# 0x4fd950, 0x4fdc60, 0x501290, 0x502430) against the originals: positions anywhere, outs
# over the fields, ticks about the wrap.
# usage: person.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=MapId,Record,TimeInPark,PushRide)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

B = Bench([(0x4e2650, 0x4e266d), (0x4f9460, 0x4f948b), (0x4fd950, 0x4fd967), (0x4fdc60, 0x4fdc89), (0x501290, 0x5012ac),
           (0x502430, 0x502454)])
THING, WORLD = RAM, RAM + 0x1000
GWORLD = 0x80239c
CALLS = {'MapId': 0x4e2650, 'Record': 0x4f9460, 'TimeInPark': 0x4fd950, 'PushRide': 0x4fdc60, 'State11': 0x501290, 'State11To14': 0x502430}


def word(r):
    return r.choice([0, 1, 4, 0x7fffffff, 0x80000000, 0xffffffff, r.getrandbits(32)])


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
    if op == 'PushRide':
        return Call(0x4fdc60, [THING, r.getrandbits(32)], regions, conv='t', ret='v')
    out = THING + r.choice([0x1f0 + r.randrange(8), 0xcb, 0xcc, 0xcd, 4, 6])  # aliasing the field or the position
    return Call(CALLS[op], [THING, out], regions, conv='t', ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
