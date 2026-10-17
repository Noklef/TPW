# A flying ride slot's arrival (src/game/world/flyingrides: 0x55d830) against the original: the flying rides bench's records (tests/fncmp/flyingrides.py
# record(): the first slot in state 1 anywhere or none, its count either side of 0, its part count and base), indices in and out of 1-99, and the ride
# part the new count names activated on the part state bench's instance (tests/fncmp/partstate.py activation(): a thing handle starting a sprite
# script), the part found by key among the model's flag 0x80 parts or not.
# usage: flyingslots.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import flyingrides  # noqa: E402
import partstate  # noqa: E402
from fncmp import HEAP, Bench, Call, RAM, heap, put  # noqa: E402

B = Bench(partstate.RANGES + [(0x55d830, 0x55d945), (0x44b220, 0x44b267), (0x475550, 0x47558b)])
HOLDER, MODEL, PARTS, REC = RAM + 0x20000, RAM + 0x20100, RAM + 0x20200, RAM + 0x21000


def case(r):
    regions = {HEAP: heap()}
    partstate.activation(r, regions)
    instance = regions[partstate.INSTANCE]
    put(instance, 4, 'I', MODEL)
    index = r.choice([1, 2, 99, r.randrange(1, 100)] * 5 + [0, 100, -1])
    table = bytearray(r.bytes(0x194))
    if 0 < index <= 100:
        put(table, index * 4, 'I', REC if r.random() < 0.95 else 0)
    rides = bytearray(r.bytes(0x10))
    for k in range(1, 4):
        put(rides, k * 4, 'I', HOLDER)
    model = bytearray(r.bytes(0x100))
    put(model, 0x48, 'H', r.choice([0, 1, 4, 8]))
    put(model, 0x7c, 'I', PARTS)
    parts = bytearray(r.bytes(0xa0))
    for i in range(8):
        put(parts, i * 0x14, '2I', r.choice([0x80, 0x80, 0x81, 1, 0, r.getrandbits(32)]), r.choice([1, 2, 3, 4, 5, 8]))
    regions.update({0x8791f8: table, 0x7a4610: rides, HOLDER: put(bytearray(r.bytes(0x40)), 8, 'I', partstate.INSTANCE), MODEL: model,
                    PARTS: parts, REC: flyingrides.record(r, 'In')})
    handle = r.choice([0, 1, 2, 3, 3]) | (r.getrandbits(16) << 16 if r.random() < 0.3 else 0)
    return Call(0x55d830, [index & 0xffffffff, handle], regions, ret='a')


if __name__ == '__main__':
    B.main(case)
