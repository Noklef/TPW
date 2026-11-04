# A ride's animation stepped, alone or all of them (src/game/coaster/rideanimate: 0x44e380 Ride_AnimateOne, 0x44e410
# Rides_AnimateAll) against the originals: the animator's update (k 1), aim sprites (k 2), strip step (k 3) and layer reset
# (k 4) recorders, the flags they leave.
# usage: rideanimate.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, log_region, put  # noqa: E402

UPDATE, AIM, STRIP, RESET = 1, 2, 3, 4
B = Bench([(0x44e380, 0x44e407), (0x44e410, 0x44e503)],
          hooks={0x473c70: _recorder(UPDATE, 3, cdecl=True), 0x44b510: _recorder(AIM, 1, cdecl=True),
                 0x4548f0: _recorder(STRIP, 2, cdecl=True), 0x4726d0: _recorder(RESET, 1, cdecl=True)})
RIDES, PARTS = RAM, RAM + 0x1000
SIZE = 0x40
OPS = [k for k in os.environ.get('OPS', 'one,all').split(',') if k]


def ride(r, i):
    m = bytearray(r.bytes(SIZE))
    flags = r.getrandbits(32) & ~0x883020 & ~0x200000
    for bit in (0x800000, 0x80000, 0x20, 0x1000, 0x2000):
        if r.random() < (0.25 if bit & 0x3000 else 0.5):
            flags |= bit
    if r.random() < 0.3:
        flags |= 0x200000
    put(m, 4, 'I', flags)
    put(m, 8, 'I', PARTS + 0x40 * i)
    put(m, 0x14, 'I', r.choice([0, r.getrandbits(32) | 1, r.getrandbits(32) | 1]))
    return bytes(m)


def part(r):
    m = bytearray(r.bytes(0x40))
    put(m, 0x28, 'I', r.choice([0, r.getrandbits(32) | 1, 1, 0x100]))
    return bytes(m)


def case(r):
    regions = {LOG: log_region(), 0x7b497c: r.bytes(4)}
    count = 8
    for i in range(count):
        regions[RIDES + SIZE * i] = ride(r, i)
        regions[PARTS + 0x40 * i] = part(r)
    if r.deal(OPS) == 'one':
        return Call(0x44e380, [RIDES, r.choice([0, 0, 1, r.getrandbits(32) | 2])], regions)
    mode = r.choice([0, 1, 2, 2, 3, r.getrandbits(32)])
    regions[0x790ab0] = struct.pack('<I', r.getrandbits(32) & ~4 | r.choice([0, 4, 4]))
    regions[0x790988] = struct.pack('<I', RIDES + SIZE * r.randrange(count))
    first = r.randrange(0, 5)
    regions[0x7a3ebc] = struct.pack('<I', first)
    regions[0x7a4084] = struct.pack('<I', r.randrange(0, 9 - first))
    regions[0x7a4614] = struct.pack('<9I', *[r.choice([0, RIDES + SIZE * i, RIDES + SIZE * i]) for i in range(8)], r.getrandbits(32))
    return Call(0x44e410, [mode], regions)


if __name__ == '__main__':
    B.main(case)
