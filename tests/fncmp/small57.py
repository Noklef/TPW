# The litter removal of src/game/core/small57 (0x4d9620) against the original: litter of kind 7, 8 and others on cells near the start of the map, with the spread records
# (mapstep.py's world), a particle system handle that is live or stale (particles.py's systems), and the sprite scripts not ready (the kill is then a no-op).
# usage: small57.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

B = Bench([(0x4d9620, 0x4d96f0), (0x4d8440, 0x4d8620), (0x51feb0, 0x51ff10), (0x5222e0, 0x522320), (0x475550, 0x47558b), (0x5da3c0, 0x5da3c1)])
WORLD = RAM + 0x10000


def case(r):
    world = struct.pack('<I', WORLD)
    records = bytearray(r.bytes(12 * 8))
    for k in range(8):
        records[12 * k + 0xa] = r.choice([0, 1, 2, 3, 3])
    litter = bytearray(r.bytes(0x60))
    put(litter, 0, 'H', r.choice([1, 2, 0x80, 0x81, 0x100, 0x14a, r.randrange(1, 0x100)]))
    put(litter, 0x2c, 'I', r.choice([7, 8, 7, 8, 0, 1, 9]))
    put(litter, 0x38, 'I', r.choice([0, r.getrandbits(32), 1, (r.getrandbits(16) << 16) | r.randrange(0x78)]))
    put(litter, 0x28, 'I', r.choice([0, 1, 5]))
    setup = put(bytearray(r.bytes(0x10)), 0, 'I', r.choice([0, 1, 1, 1]))
    regions = {RAM: litter, 0x8023a0: world, WORLD + 0x1d9104: records, WORLD + 0x1b1104: r.bytes(10 * 0x302), 0x7b49f0: bytearray(16), 0x816d40: setup,
               0x80cefc: r.bytes(0x78 * 0x140)}
    return Call(0x4d9620, [RAM], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
