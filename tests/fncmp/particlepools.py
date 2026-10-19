# The particle pools' resize (src/game/render/particles: 0x5ed5a0) against the original, on the particle bench's Spawn world (tests/fncmp/particles.py
# spawn()): pools of 16 handles at 0xf8692c (0x44 bytes each: the count, then the handles) grown (each new handle Particle_Spawn's of the effect) or
# shrunk (each handle past the new count ended, ParticleSystem_End, and cleared when there is one), or left; handles of the six systems (their serials
# right or not), none or random. Also pool 2's emptying (0x5edac0) and its marking then emptying (0x5eda70). And a pool's move (0x5ed660:
# the position blocks at 0xf86900, 0x44 apart, of random floats; the old position kept or jumped; a third float 0, -0 or not, with the game
# clock's real time less its pauses as gameclock.py's less_pauses() sets it up). Pool 2's set-up for an area (0x5ed920): see case().
# usage: particlepools.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import particles  # noqa: E402
from fncmp import Bench, Call, fb, put  # noqa: E402
from gameclock import less_pauses  # noqa: E402

B = Bench([(0x51feb0, 0x51ff70), (0x520560, 0x520e10), (0x521d60, 0x5222f0), (0x522360, 0x522500), (0x5ed5a0, 0x5ed670), (0x5eda70, 0x5edad0), (0x5da3c0, 0x5da3c1),
           (0x5ed660, 0x5ed6f1), (0x402d70, 0x402e00), (0x402e60, 0x4030f0), (0x403150, 0x403172), (0x4031e0, 0x403202), (0x4033a0, 0x4033f0),
           (0x5f5e20, 0x5f5e80), (0x5f5f10, 0x5f5fc0), (0x5988e0, 0x598955), (0x5ed920, 0x5eda36), (0x67a830, 0x67a860)])
POOLS = 0xf8692c
OPS = [k for k in os.environ.get('OPS', 'Resize,Finish,Empty,Move,Setup').split(',') if k]


def move(r):   # 0x5ed660 (pool, jump, x, y, t): +0 the old position (x, y if jumping, else the last), +8 the new; when t is not 0 the clock's time at
    c = less_pauses(r)   # +0x10 (as a float) and t at +0x14
    index = r.choice([0, 1, 2, 3])
    regions = dict(c.regions)
    regions[0xf86900] = bytes(r.floats(bytearray(r.bytes(0x44 * 4 + 0x20)), range(0, 0x44 * 4, 4)))
    odd = lambda: r.choice([0x7fc00000, 0xffc00000, 0x7f800001, 0xff812345, 0x7f800000])  # noqa: E731   (NaNs quiet and signalling)
    t = r.choice([0, 0, 0x80000000, odd(), r.f(), r.f()])
    return Call(0x5ed660, [index, r.choice([0, 1, r.getrandbits(32)]), r.choice([r.f(), r.f(), odd()]), r.choice([r.f(), r.f(), odd()]), t], regions, ret='v')


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

    op = r.deal(OPS)
    if op == 'Move':
        return move(r)
    index = r.choice([0, 1, 2, 3]) if op == 'Resize' else 2
    pools = bytearray(r.bytes(0x44 * (index + 2)))
    old = r.choice([0, 1, 2, 5, 8, 16, r.randrange(17)])
    new = r.choice([0, 1, 2, 5, 8, 16, old, old + 1, max(old - 1, 0), r.randrange(17)])
    put(pools, 0x44 * index, 'i', old)
    for k in range(16):
        put(pools, 0x44 * index + 4 + 4 * k, 'I', handle())
    regions[POOLS] = pools
    if op == 'Setup':  # pool 2's count from the area width * height: sizes about the counts' steps (0.0016 + 0.0051 / 3 apart) or anything; the
        # position block (0xf86900 on) random under the pools, the clock as for a move
        regions.update(less_pauses(r).regions)
        regions[0xf86900] = bytes(r.bytes(0x2c)) + bytes(regions.pop(POOLS))
        side = lambda: r.choice([fb(r.uniform(0.03, 0.1)), fb(r.uniform(0, 0.2)), fb(r.uniform(-0.1, 0.1)), r.f(), fb(0.0), fb(-0.0)])  # noqa: E731
        w = side()
        h = r.choice([side(), w])
        return Call(0x5ed920, [r.f(), r.f(), w, h], regions, ret='v')
    effect = c.args[0] if r.random() < 0.9 else r.choice([0, 1, 0x68])
    if op != 'Resize':  # pool 2 emptied, the first marking its systems (ParticleSystem_Set2a) first
        return Call({'Finish': 0x5eda70, 'Empty': 0x5edac0}[op], [], regions, ret='a')
    return Call(0x5ed5a0, [index, new, effect], regions, ret='a')


if __name__ == '__main__':
    B.main(case)
