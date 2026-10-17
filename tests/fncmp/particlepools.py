# The particle pools' resize (src/game/render/particles: 0x5ed5a0) against the original, on the particle bench's Spawn world (tests/fncmp/particles.py
# spawn()): pools of 16 handles at 0xf8692c (0x44 bytes each: the count, then the handles) grown (each new handle Particle_Spawn's of the effect) or
# shrunk (each handle past the new count ended, ParticleSystem_End, and cleared when there is one), or left; handles of the six systems (their serials
# right or not), none or random.
# usage: particlepools.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import particles  # noqa: E402
from fncmp import Bench, Call, put  # noqa: E402

B = Bench([(0x51feb0, 0x51ff20), (0x520560, 0x520e10), (0x521d60, 0x5222f0), (0x522360, 0x522500), (0x5ed5a0, 0x5ed670), (0x5da3c0, 0x5da3c1)])
POOLS = 0xf8692c


def case(r):
    c = particles.spawn(r)
    regions = dict(c.regions)
    systems = regions[0x80cefc]  # (the set-up flag 0x816d40 is in spawn()'s heads)
    defs = regions[0x804370]
    for k in range(0x69):  # no voice links: a pool's second voice start would mark an instance past the table by its counter (a fault the driver's
        if defs[0x140 * k + 0xba]:  # memory hides)
            defs[0x140 * k + 0xba] = 0
            put(defs, 0x140 * k + 0xb4, 'i', -1)

    def handle():
        slot = r.randrange(6)
        serial = struct.unpack_from('<H', systems, 0x140 * slot + 0xa)[0]
        return r.choice([serial << 16 | slot] * 4 + [0, 0, (serial ^ 1) << 16 | slot, r.getrandbits(32)])

    index = r.choice([0, 1, 2, 3])
    pools = bytearray(r.bytes(0x44 * (index + 2)))
    old = r.choice([0, 1, 2, 5, 8, 16, r.randrange(17)])
    new = r.choice([0, 1, 2, 5, 8, 16, old, old + 1, max(old - 1, 0), r.randrange(17)])
    put(pools, 0x44 * index, 'i', old)
    for k in range(16):
        put(pools, 0x44 * index + 4 + 4 * k, 'I', handle())
    regions[POOLS] = pools
    effect = c.args[0] if r.random() < 0.9 else r.choice([0, 1, 0x68])
    return Call(0x5ed5a0, [index, new, effect], regions, ret='a')


if __name__ == '__main__':
    B.main(case)
